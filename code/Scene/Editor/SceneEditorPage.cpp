#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyArray.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Resource/ResourceManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityClipboardData.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/ISceneControllerEditorFactory.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/Events/CameraMovedEvent.h"
#include "Scene/Editor/Events/PostBuildEvent.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Scene/Editor/Events/PostModifyEvent.h"
#include "Scene/Editor/Events/PreModifyEvent.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptFactory.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/InputDialog.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/GridView/GridView.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridColumnClickEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowStateChangeEvent.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/EntityEventManager.h"
#include "World/IEntityComponent.h"
#include "World/IEntityComponentData.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

bool isChildEntitySelected(const EntityAdapter* entityAdapter)
{
	for (auto child : entityAdapter->getChildren())
	{
		if (child->isSelected())
			return true;
		if (isChildEntitySelected(child))
			return true;
	}
	return false;
}

bool filterIncludeEntity(const TypeInfo& entityOrComponentType, EntityAdapter* entityAdapter)
{
	if (!entityAdapter->getEntity())
		return true;

	if (is_type_of(entityOrComponentType, type_of(entityAdapter->getEntity())))
		return true;

	if (entityAdapter->getComponent(entityOrComponentType) != nullptr)
		return true;

	for (auto child : entityAdapter->getChildren())
	{
		if (filterIncludeEntity(entityOrComponentType, child))
			return true;
	}

	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorPage", SceneEditorPage, editor::IEditorPage)

SceneEditorPage::SceneEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_entityFilterType(nullptr)
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	std::set< std::wstring > guideIds;

	// Get render system from store.
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	// Create world event manager.
	Ref< world::EntityEventManager > eventManager = new world::EntityEventManager(64);

	// Get physics manager type.
	std::wstring physicsManagerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName.c_str());
	if (!physicsManagerType)
	{
		log::error << L"Unable to create scene editor; no such physics manager type \"" << physicsManagerTypeName << L"\"." << Endl;
		return false;
	}

	// Create physics manager.
	physics::PhysicsCreateDesc pcd;
	pcd.timeScale = 1.0f;
	pcd.solverIterations = 10;

	Ref< physics::PhysicsManager > physicsManager = checked_type_cast< physics::PhysicsManager* >(physicsManagerType->createInstance());
	if (!physicsManager->create(pcd))
	{
		log::error << L"Unable to create scene editor; failed to create physics manager." << Endl;
		return false;
	}

	// Configure physics manager.
	physicsManager->setGravity(Vector4(0.0f, -9.81f, 0.0f, 0.0f));

	// Create script context.
	Ref< script::IScriptManager > scriptManager = m_editor->getStoreObject< script::IScriptManager >(L"ScriptManager");
	if (!scriptManager)
	{
		log::error << L"Unable to create scene editor; failed to get script manager." << Endl;
		return false;
	}

	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(false);
	if (!scriptContext)
	{
		log::error << L"Unable to create scene editor; failed to create script context." << Endl;
		return false;
	}

	// Create resource manager.
	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(
		m_editor->getOutputDatabase(),
		m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false)
	);

	// Create editor context.
	m_context = new SceneEditorContext(
		m_editor,
		m_document,
		m_editor->getOutputDatabase(),
		m_editor->getSourceDatabase(),
		eventManager,
		resourceManager,
		renderSystem,
		physicsManager,
		scriptContext
	);

	// Create profiles, plugins, resource factories, entity editors and guide ids.
	TypeInfoSet profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (auto profileType : profileTypes)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >(profileType->createInstance());
		if (!profile)
			continue;

		m_context->addEditorProfile(profile);

		RefArray< ISceneEditorPlugin > editorPlugins;
		profile->createEditorPlugins(m_context, editorPlugins);

		for (auto editorPlugin : editorPlugins)
			m_context->addEditorPlugin(editorPlugin);

		RefArray< const resource::IResourceFactory > resourceFactories;
		profile->createResourceFactories(m_context, resourceFactories);

		for (auto resourceFactory : resourceFactories)
			resourceManager->addFactory(resourceFactory);

		profile->getGuideDrawIds(guideIds);
	}

	// Create entity and component editor factories.
	m_context->createFactories();

	// Create scene instance resource factory, used by final render control etc.
	RefArray< const world::IEntityFactory > entityFactories;

	for (auto editorProfile : m_context->getEditorProfiles())
		editorProfile->createEntityFactories(m_context, entityFactories);

	Ref< world::EntityBuilder > entityBuilder = new world::EntityBuilder();
	for (auto entityFactory : entityFactories)
		entityBuilder->addFactory(entityFactory);

	m_context->getResourceManager()->addFactory(
		new SceneFactory(m_context->getRenderSystem(), entityBuilder)
	);

	// Create editor panel.
	m_editPanel = new ui::Container();
	m_editPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0));

	m_editControl = new ScenePreviewControl();
	if (!m_editControl->create(m_editPanel, m_context))
	{
		log::error << L"Unable to create scene editor; failed to scene preview." << Endl;
		return false;
	}

	m_statusBar = new ui::StatusBar();
	m_statusBar->create(m_editPanel, ui::WsDoubleBuffer);

	// Create entity panel.
	m_entityPanel = new ui::Container();
	m_entityPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenuDefault = new ui::Menu();
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));
	m_entityMenuDefault->add(new ui::MenuItem(L"-"));
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Scene.Editor.CreateExternal"), i18n::Text(L"SCENE_EDITOR_CREATE_EXTERNAL")));
	m_entityMenuDefault->add(new ui::MenuItem(L"-"));
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_entityMenuGroup = new ui::Menu();
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));
	m_entityMenuGroup->add(new ui::MenuItem(L"-"));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.CreateExternal"), i18n::Text(L"SCENE_EDITOR_CREATE_EXTERNAL")));
	m_entityMenuGroup->add(new ui::MenuItem(L"-"));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddGroupEntity"), i18n::Text(L"SCENE_EDITOR_ADD_GROUP_ENTITY")));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_entityMenuExternal = new ui::Menu();
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(L"-"));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.ResolveExternal"), i18n::Text(L"SCENE_EDITOR_RESOLVE_EXTERNAL")));
	m_entityMenuExternal->add(new ui::MenuItem(L"-"));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.FindInDatabase"), i18n::Text(L"SCENE_EDITOR_FIND_IN_DATABASE")));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_entityToolBar = new ui::ToolBar();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.RemoveEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveToEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.FilterEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveUpEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveDownEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.AddLayer"), 1);
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), 0, ui::Command(L"Editor.Delete")));
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY"), 1, ui::Command(L"Scene.Editor.MoveToEntity")));
	m_buttonFilterEntity = new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_FILTER_ENTITY"), 2, ui::Command(L"Scene.Editor.FilterEntity"), ui::ToolBarButton::BsDefaultToggle);
	m_entityToolBar->addItem(m_buttonFilterEntity);
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_UP"), 3, ui::Command(L"Scene.Editor.MoveUp")));
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_DOWN"), 4, ui::Command(L"Scene.Editor.MoveDown")));
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_NEW_LAYER"), 5, ui::Command(L"Scene.Editor.NewLayer")));
	m_entityToolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SceneEditorPage::eventEntityToolClick);

	m_imageHidden = new ui::StyleBitmap(L"Scene.LayerHidden");
	m_imageVisible = new ui::StyleBitmap(L"Scene.LayerVisible");
	m_imageLocked = new ui::StyleBitmap(L"Scene.LayerLocked");
	m_imageUnlocked = new ui::StyleBitmap(L"Scene.LayerUnlocked");

	m_instanceGrid = new ui::GridView();
	m_instanceGrid->create(m_entityPanel, ui::GridView::WsMultiSelect | ui::GridView::WsAutoEdit | ui::WsDoubleBuffer);
	m_instanceGrid->addColumn(new ui::GridColumn(L"", ui::dpi96(200), true));
	m_instanceGrid->addColumn(new ui::GridColumn(L"", ui::dpi96(30)));
	m_instanceGrid->addColumn(new ui::GridColumn(L"", ui::dpi96(30)));
	m_instanceGrid->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventInstanceSelect);
	m_instanceGrid->addEventHandler< ui::GridRowStateChangeEvent >(this, &SceneEditorPage::eventInstanceExpand);
	m_instanceGrid->addEventHandler< ui::MouseButtonDownEvent >(this, &SceneEditorPage::eventInstanceButtonDown);
	m_instanceGrid->addEventHandler< ui::GridColumnClickEvent >(this, &SceneEditorPage::eventInstanceClick);
	m_instanceGrid->addEventHandler< ui::GridItemContentChangeEvent >(this, &SceneEditorPage::eventInstanceRename);

	m_instanceGridFontBold = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontBold->setBold(true);

	m_instanceGridFontHuge = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontHuge->setSize(12);

	m_site->createAdditionalPanel(m_entityPanel, ui::dpi96(300), false);

	m_tabMisc = new ui::Tab();
	m_tabMisc->create(parent, ui::Tab::WsLine);
	m_tabMisc->setText(i18n::Text(L"SCENE_EDITOR_MISC"));

	// Create dependency panel.
	Ref< ui::TabPage > tabPageDependencies = new ui::TabPage();
	tabPageDependencies->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"), new ui::FloodLayout());

	m_entityDependencyPanel = new EntityDependencyInvestigator(m_context);
	m_entityDependencyPanel->create(tabPageDependencies);

	// Create guide visibility panel.
	Ref< ui::TabPage > tabPageGuides = new ui::TabPage();
	tabPageGuides->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_GUIDES"), new ui::FloodLayout());

	m_gridGuides = new ui::GridView();
	m_gridGuides->create(tabPageGuides, ui::WsDoubleBuffer | ui::WsTabStop);
	m_gridGuides->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_NAME"), ui::dpi96(150)));
	m_gridGuides->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_VISIBLE"), ui::dpi96(50)));

	for (const auto& guideId : guideIds)
	{
		bool shouldDraw = m_editor->getSettings()->getProperty< bool >(L"SceneEditor.Guides/" + guideId, true);
		m_context->setDrawGuide(guideId, shouldDraw);

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(guideId));
		row->add(new ui::GridItem(shouldDraw ? m_imageVisible : m_imageHidden));
		m_gridGuides->addRow(row);
	}

	m_gridGuides->addEventHandler< ui::GridColumnClickEvent >(this, &SceneEditorPage::eventGuideClick);

	// Add pages.
	m_tabMisc->addPage(tabPageDependencies);
	m_tabMisc->addPage(tabPageGuides);
	m_tabMisc->setActivePage(tabPageDependencies);

	m_site->createAdditionalPanel(m_tabMisc, ui::dpi96(300), false);

	// Create controller panel.
	m_controllerPanel = new ui::Container();
	m_controllerPanel->create(parent, ui::WsNone, new ui::FloodLayout());
	m_controllerPanel->setText(i18n::Text(L"SCENE_EDITOR_CONTROLLER"));

	m_site->createAdditionalPanel(m_controllerPanel, ui::dpi96(120), true);

	// Context event handlers.
	m_context->addEventHandler< PostBuildEvent >(this, &SceneEditorPage::eventContextPostBuild);
	m_context->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventContextSelect);
	m_context->addEventHandler< PreModifyEvent >(this, &SceneEditorPage::eventContextPreModify);
	m_context->addEventHandler< PostModifyEvent >(this, &SceneEditorPage::eventContextPostModify);
	m_context->addEventHandler< CameraMovedEvent >(this, &SceneEditorPage::eventContextCameraMoved);
	m_context->addEventHandler< PostFrameEvent >(this, &SceneEditorPage::eventContextPostFrame);

	// Load last used camera transforms.
	for (int32_t i = 0; i < 4; ++i)
	{
		std::wstring str = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.LastCameras/" + m_context->getDocument()->getInstance(0)->getGuid().format() + L"/" + toString(i));
		std::vector< float > values;
		Split< std::wstring, float >::any(str, L",", values);
		if (values.size() >= 7)
		{
			m_context->getCamera(i)->setPosition(Vector4(values[0], values[1], values[2], 1.0f));
			m_context->getCamera(i)->setOrientation(Quaternion(values[3], values[4], values[5], values[6]));
		}
	}

	// Finally realize the scene.
	createSceneAsset();
	updateScene();
	createInstanceGrid();
	createControllerEditor();
	updatePropertyObject();
	updateStatusBar();
	return true;
}

void SceneEditorPage::destroy()
{
	// Save cameras.
	auto settings = m_editor->checkoutGlobalSettings();
	for (int32_t i = 0; i < 4; ++i)
	{
		auto p = m_context->getCamera(i)->getPosition();
		auto o = m_context->getCamera(i)->getOrientation();
		settings->setProperty< PropertyString >(
			L"SceneEditor.LastCameras/" + m_context->getDocument()->getInstance(0)->getGuid().format() + L"/" + toString(i),
			str(L"%f, %f, %f, %f, %f, %f, %f", (float)p.x(), (float)p.y(), (float)p.z(), (float)o.e.x(), (float)o.e.y(), (float)o.e.z(), (float)o.e.w())
		);
	}
	m_editor->commitGlobalSettings();

	// Destroy controller editor.
	if (m_context->getControllerEditor())
		m_context->getControllerEditor()->destroy();

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_entityPanel);
	m_site->destroyAdditionalPanel(m_tabMisc);
	m_site->destroyAdditionalPanel(m_controllerPanel);

	// Destroy widgets.
	safeDestroy(m_editPanel);
	safeDestroy(m_editControl);
	safeDestroy(m_entityPanel);
	safeDestroy(m_tabMisc);
	safeDestroy(m_controllerPanel);
	safeDestroy(m_entityToolBar);
	safeDestroy(m_instanceGrid);

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();

	safeDestroy(m_context);
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	// Get index of view where user dropped instance.
	uint32_t viewIndex;
	if (!m_editControl->getViewIndex(position, viewIndex))
		return false;

	Ref< world::EntityData > entityData;

	// Check profiles if any can convert instance into an entity data.
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		if ((entityData = editorProfile->createEntityData(m_context, instance)) != nullptr)
			break;
	}

	if (entityData)
	{
		Ref< EntityAdapter > parentGroupAdapter;

		// Get selected items, must be a single item.
		RefArray< ui::GridRow > selectedRows;
		if (m_instanceGrid->getRows(selectedRows, ui::GridView::GfDescendants | ui::GridView::GfSelectedOnly) == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT(selectedEntity);

			parentGroupAdapter = selectedEntity->getParentGroup();
		}

		// Ensure drop is valid.
		if (!parentGroupAdapter)
		{
			log::error << L"Unable to drop entity; no layer or group selected" << Endl;
			return false;
		}
		if (parentGroupAdapter->isLocked(true))
		{
			log::error << L"Unable to drop entity; layer or group is locked" << Endl;
			return false;
		}

		// Ensure group is selected when editing a prefab.
		Object* documentObject = m_context->getDocument()->getObject(0);
		T_ASSERT(documentObject);

		if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(documentObject))
		{
			if (parentGroupAdapter->isLayer())
			{
				log::error << L"Unable to drop entity; no prefab group selected" << Endl;
				return false;
			}
		}

		// Issue automatic build of dropped entity just in case the
		// entity hasn't been built.
		if (m_editor->getSettings()->getProperty< bool >(L"SceneEditor.BuildWhenDrop", true))
			m_editor->buildAsset(instance->getGuid(), false);

		m_context->getDocument()->push();

		// Create instance and adapter.
		Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
		entityAdapter->prepare(entityData, 0, 0);

		// Place instance in front of perspective camera.
		const Camera* camera = m_context->getCamera(viewIndex);
		T_ASSERT(camera);

		Matrix44 Mworld = camera->getWorld().toMatrix44() * translate(0.0f, 0.0f, 4.0f);
		entityAdapter->setTransform(Transform(Mworld.translation()));

		// Finally add adapter to parent group.
		parentGroupAdapter->addChild(entityAdapter);

		updateScene();
		createInstanceGrid();

		// Select entity.
		m_context->selectAllEntities(false);
		m_context->selectEntity(entityAdapter);
		m_context->raiseSelect();
	}
	else
		return false;

	return true;
}

bool SceneEditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.PropertiesChanging")
		m_context->getDocument()->push();
	if (command == L"Editor.PropertiesChanged")
	{
		updateScene();
		createInstanceGrid();

		// Notify controller editor as well.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->propertiesChanged();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_context->getDocument()->undo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_context->getDocument()->redo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

		// Create clipboard data with all selected entities; remove entities from scene if we're cutting.
		Ref< EntityClipboardData > entityClipboardData = new EntityClipboardData();
		for (auto selectedEntity : selectedEntities)
		{
			entityClipboardData->addEntityData(selectedEntity->getEntityData());
			if (command == L"Editor.Cut")
			{
				Ref< EntityAdapter > parentGroup = selectedEntity->getParent();
				if (parentGroup->isGroup())
				{
					parentGroup->removeChild(selectedEntity);

					if (m_context->getControllerEditor())
						m_context->getControllerEditor()->entityRemoved(selectedEntity);
				}
			}
		}

		ui::Application::getInstance()->getClipboard()->setObject(entityClipboardData);

		if (command == L"Editor.Cut")
		{
			updateScene();
			createInstanceGrid();

			m_context->raiseSelect();
		}
	}
	else if (command == L"Editor.Paste")
	{
		// Get parent group under which we will place the new entity.
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
			return false;

		Ref< EntityAdapter > parentEntity = selectedEntities[0]->getParentGroup();
		T_ASSERT(parentEntity);

		// Get clipboard data; ensure correct type.
		Ref< EntityClipboardData > entityClipboardData = dynamic_type_cast< EntityClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!entityClipboardData)
			return false;

		const RefArray< world::EntityData >& entityDatas = entityClipboardData->getEntityData();
		if (entityDatas.empty())
			return false;

		m_context->getDocument()->push();

		// Create new instances and adapters for each entity found in clipboard.
		for (auto entityData : entityDatas)
		{
			Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
			entityAdapter->prepare(entityData, 0, 0);
			parentEntity->addChild(entityAdapter);
		}

		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

		uint32_t removedCount = 0;
		for (auto selectedEntity : selectedEntities)
		{
			Ref< EntityAdapter > parentGroup = selectedEntity->getParent();
			if (parentGroup && parentGroup->isGroup())
			{
				parentGroup->removeChild(selectedEntity);
				removedCount++;

				if (m_context->getControllerEditor())
					m_context->getControllerEditor()->entityRemoved(selectedEntity);
			}
			else if (selectedEntity->isLayer())
			{
				RefArray< world::LayerEntityData > layers = m_context->getSceneAsset()->getLayers();
				layers.remove(checked_type_cast< world::LayerEntityData*, false >(selectedEntity->getEntityData()));
				m_context->getSceneAsset()->setLayers(layers);
				removedCount++;
			}
		}

		if (removedCount)
		{
			updateScene();
			createInstanceGrid();

			m_context->raiseSelect();
		}
	}
	else if (command == L"Editor.SelectAll")
	{
		m_context->selectAllEntities();
		m_context->raiseSelect();
	}
	else if (command == L"Scene.Editor.CreateExternal")
		result = createExternal();
	else if (command == L"Scene.Editor.ResolveExternal")
		result = resolveExternal();
	else if (command == L"Scene.Editor.AddEntity")
		result = addEntity(nullptr);
	else if (command == L"Scene.Editor.AddGroupEntity")
		result = addEntity(&type_of< world::GroupEntityData >());
	else if (command == L"Scene.Editor.MoveToEntity")
		result = moveToEntity();
	else if (command == L"Scene.Editor.MoveUp")
	{
		if ((result = moveUp()) == true)
		{
			updateScene();
			createInstanceGrid();
		}
	}
	else if (command == L"Scene.Editor.MoveDown")
	{
		if ((result = moveDown()) == true)
		{
			updateScene();
			createInstanceGrid();
		}
	}
	else if (command == L"Scene.Editor.NewLayer")
	{
		m_context->getDocument()->push();

		auto layers = m_context->getSceneAsset()->getLayers();
		layers.push_back(new world::LayerEntityData());
		m_context->getSceneAsset()->setLayers(layers);

		updateScene();
		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.FilterEntity")
	{
		if (m_buttonFilterEntity->isToggled())
			m_entityFilterType = m_editor->browseType(
				makeTypeInfoSet< world::Entity, world::IEntityComponent >(),
				false,
				false
			);
		else
			m_entityFilterType = nullptr;

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.EnlargeGuide")
	{
		float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(guideSize + 0.5f);
	}
	else if (command == L"Scene.Editor.ShrinkGuide")
	{
		float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(std::max(guideSize - 0.5f, 0.5f));
	}
	else if (command == L"Scene.Editor.ResetGuide")
		m_context->setGuideSize(1.0f);
	else if (command == L"Scene.Editor.FindInDatabase")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
			return false;

		Guid externalGuid;
		if (selectedEntities[0]->getExternalGuid(externalGuid))
		{
			Ref< db::Instance > externalInstance = m_context->getEditor()->getSourceDatabase()->getInstance(externalGuid);
			if (externalInstance)
				m_context->getEditor()->highlightInstance(externalInstance);
		}
	}
	else if (command == L"Scene.Editor.LockEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setLocked(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockAllEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.ShowEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setVisible(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.ShowAllEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setVisible(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.HideEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (auto selectedEntity : selectedEntities)
			selectedEntity->setVisible(false);

		createInstanceGrid();
	}
	else
	{
		result = false;

		// Propagate command to controller editor.
		if (!result && m_context->getControllerEditor())
			result = m_context->getControllerEditor()->handleCommand(command);

		// Propagate command to editor control.
		if (!result)
			result = m_editControl->handleCommand(command);
	}

	return result;
}

void SceneEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (!m_context || database == m_editor->getSourceDatabase())
		return;

	// Flush resource from manager.
	m_context->getResourceManager()->reload(eventId, false);

	// Check if guid is used as an external reference.
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	bool externalModified = false;
	for (auto entityAdapter : entityAdapters)
	{
		Guid externalGuid;
		if (entityAdapter->getExternalGuid(externalGuid))
		{
			if (externalGuid == eventId)
			{
				// Modified external entity detected; need to recreate the scene.
				// Also drop hash to ensure external entity is recreated, hash doesn't include hash of external data.
				externalModified = true;
				entityAdapter->dropHash();
				break;
			}
		}
	}

	if (externalModified)
	{
		updateScene();
		createInstanceGrid();
	}
}

bool SceneEditorPage::createSceneAsset()
{
	Object* documentObject = m_context->getDocument()->getObject(0);
	if (!documentObject)
		return false;

	if (auto sceneAsset = dynamic_type_cast< SceneAsset* >(documentObject))
		m_context->setSceneAsset(sceneAsset);
	else if (auto entityData = dynamic_type_cast< world::EntityData* >(documentObject))
	{
		Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
		if (!sceneAsset)
			return false;

		const auto& layers = sceneAsset->getLayers();
		T_ASSERT(layers.size() >= 2);

		layers[1]->addEntityData(entityData);

		m_context->setSceneAsset(sceneAsset);
	}
	else
		return false;

	return true;
}

void SceneEditorPage::createControllerEditor()
{
	if (m_context->getControllerEditor())
	{
		m_context->getControllerEditor()->destroy();
		m_context->setControllerEditor(nullptr);
	}

	m_site->hideAdditionalPanel(m_controllerPanel);

	Ref< SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		Ref< ISceneControllerData > controllerData = sceneAsset->getControllerData();
		if (controllerData)
		{
			RefArray< const ISceneControllerEditorFactory > controllerEditorFactories;
			Ref< ISceneControllerEditor > controllerEditor;

			// Create controller editor factories.
			for (auto profile : m_context->getEditorProfiles())
				profile->createControllerEditorFactories(m_context, controllerEditorFactories);

			for (auto controllerEditorFactory : controllerEditorFactories)
			{
				TypeInfoSet typeSet = controllerEditorFactory->getControllerDataTypes();
				if (typeSet.find(&type_of(controllerData)) != typeSet.end())
				{
					controllerEditor = controllerEditorFactory->createControllerEditor(type_of(controllerData));
					if (controllerEditor)
						break;
				}
			}

			if (controllerEditor)
			{
				if (controllerEditor->create(
					m_context,
					m_controllerPanel
				))
				{
					m_context->setControllerEditor(controllerEditor);
					m_site->showAdditionalPanel(m_controllerPanel);
				}
				else
					log::error << L"Unable to create controller editor; create failed" << Endl;
			}
			else
				T_DEBUG(L"Unable to find controller editor for type \"" << type_name(controllerData) << L"\"");
		}
	}

	m_controllerPanel->update();
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		// Check if any scene settings has changed.
		bool needUpdate = false;

		DeepHash hash(sceneAsset->getWorldRenderSettings());
		if (hash != m_currentHash)
		{
			needUpdate = true;
			m_currentHash = hash.get();
		}

		// Inform editor controls to update their world renderer.
		if (needUpdate)
			m_editControl->updateWorldRenderer();
	}
}

Ref< ui::GridRow > SceneEditorPage::createInstanceGridRow(EntityAdapter* entityAdapter)
{
	if (m_entityFilterType && !filterIncludeEntity(*m_entityFilterType, entityAdapter))
		return nullptr;

	Ref< ui::GridRow > row = new ui::GridRow(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(
		(entityAdapter->isSelected() ? ui::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::GridRow::RsExpanded : 0)
	);

	std::wstring entityName = entityAdapter->getName();
	if (entityName.empty())
		entityName = i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY");

	if (entityAdapter->isExternal())
		row->add(new ui::GridItem(entityName, m_instanceGridFontBold/*, 1*/));
	else if (entityAdapter->isLayer())
	{
		row->add(new ui::GridItem(entityName, m_instanceGridFontHuge/*, 4*/));
		row->setMinimumHeight(ui::dpi96(32));
	}
	else if (entityAdapter->isGroup())
		row->add(new ui::GridItem(entityName/*, 2, 3*/));
	else
		row->add(new ui::GridItem(entityName/*, 0*/));

	// Create "visible" check box.
	row->add(new ui::GridItem(
		entityAdapter->isVisible(false) ? m_imageVisible : m_imageHidden
	));

	// Create "locked" check box.
	row->add(new ui::GridItem(
		entityAdapter->isLocked(false) ? m_imageLocked : m_imageUnlocked
	));

	// Recursively add children.
	if (
		!entityAdapter->isExternal() &&
		!entityAdapter->isChildrenPrivate()
	)
	{
		for (auto child : entityAdapter->getChildren())
		{
			Ref< ui::GridRow > childRow = createInstanceGridRow(child);
			if (childRow)
				row->addChild(childRow);
		}
	}

	return row;
}

void SceneEditorPage::createInstanceGrid()
{
	m_instanceGrid->removeAllRows();
	for (auto layerEntityAdapter : m_context->getLayerEntityAdapters())
	{
		Ref< ui::GridRow > entityRow = createInstanceGridRow(layerEntityAdapter);
		if (entityRow)
			m_instanceGrid->addRow(entityRow);
	}
	m_instanceGrid->update();
}

void SceneEditorPage::updateInstanceGridRow(ui::GridRow* row)
{
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");

	row->setState(
		(entityAdapter->isSelected() ? ui::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::GridRow::RsExpanded : 0)
	);

	for (auto childRow : row->getChildren())
		updateInstanceGridRow(childRow);
}

void SceneEditorPage::updateInstanceGrid()
{
	for (auto row : m_instanceGrid->getRows())
		updateInstanceGridRow(row);

	m_instanceGrid->update();
}

void SceneEditorPage::updatePropertyObject()
{
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly);

	if (entityAdapters.size() == 1)
	{
		Ref< EntityAdapter > entityAdapter = entityAdapters.front();
		T_ASSERT(entityAdapter);

		m_site->setPropertyObject(entityAdapter->getEntityData());
	}
	else
		m_site->setPropertyObject(m_context->getDocument()->getObject(0));
}

void SceneEditorPage::updateStatusBar()
{
	const Camera* camera = m_context->getCamera(0);
	T_ASSERT(camera);

	Vector4 position = camera->getPosition();
	Vector4 angles = camera->getOrientation().toEulerAngles();

	StringOutputStream ss;
	ss.setDecimals(2);
	ss << position.x() << L", " << position.y() << L", " << position.z() << L"     ";
	ss << rad2deg(angles.x()) << L", " << rad2deg(angles.y()) << L", " << rad2deg(angles.z()) << L" deg" << L"     ";
	ss << m_context->getEntityCount() << L" entities" << L"     ";
	ss.setDecimals(1);
	ss << m_context->getTime() << L" (" << m_context->getTimeScale() << L")";

	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		ss << L"     " << selectedEntities[0]->getPath();

	m_statusBar->setText(ss.str());
}

bool SceneEditorPage::addEntity(const TypeInfo* entityType)
{
	Ref< EntityAdapter > parentGroupAdapter;

	// Get selected entity, must be a single item.
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		parentGroupAdapter = selectedEntities[0]->getParentGroup();

	// Ensure add is valid.
	if (!parentGroupAdapter)
		return false;
	if (parentGroupAdapter->isLocked(true))
	{
		log::error << L"Unable to add entity; layer or group is locked" << Endl;
		return false;
	}

	// Select type of entity to create.
	if (!entityType)
	{
		if ((entityType = m_context->getEditor()->browseType(makeTypeInfoSet< world::EntityData >(), false, true)) == nullptr)
			return false;
	}

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->createInstance());
	T_ASSERT(entityData);

	// Browse for first component data also.
	const TypeInfo* componentType = m_context->getEditor()->browseType(makeTypeInfoSet< world::IEntityComponentData >(), false, true);
	if (componentType)
	{
		Ref< world::IEntityComponentData > componentData = dynamic_type_cast< world::IEntityComponentData* >(componentType->createInstance());
		if (componentData)
			entityData->setComponent(componentData);
	}

	m_context->getDocument()->push();

	Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
	entityAdapter->prepare(entityData, nullptr, 0);
	parentGroupAdapter->addChild(entityAdapter);

	// Select new entity.
	m_context->selectAllEntities(false);
	m_context->selectEntity(entityAdapter);

	updateScene();
	createInstanceGrid();
	updatePropertyObject();

	return true;
}

bool SceneEditorPage::createExternal()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	Ref< db::Group > group = m_editor->browseGroup();
	if (!group)
		return false;

	auto entityData = selectedEntities[0]->getEntityData();
	
	std::wstring instanceName = entityData->getName();
	if (instanceName.empty())
		instanceName = L"Unnamed";

	Ref< db::Instance > instance = group->createInstance(instanceName);
	if (!instance)
		return false;

	instance->setObject(entityData);
	if (!instance->commit())
	{
		instance->revert();
		return false;
	}

	// \tbd Replace selected entity with external reference to entity.

	m_editor->updateDatabaseView();
	return true;
}

bool SceneEditorPage::resolveExternal()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants | SceneEditorContext::GfExternalOnly) != 1)
		return false;

	auto externalAdapter = selectedEntities.front();

	Guid externalId;
	if (!externalAdapter->getExternalGuid(externalId))
		return false;

	Ref< world::EntityData > resolvedEntityData = m_context->getSourceDatabase()->getObjectReadOnly< world::EntityData >(externalId);
	if (!resolvedEntityData)
	{
		log::error << L"Unable to resolve external; failed to read entity from database." << Endl;
		return false;
	}

	resolvedEntityData->setName(externalAdapter->getName());
	resolvedEntityData->setTransform(externalAdapter->getTransform0());

	auto parent = externalAdapter->getParent();
	T_FATAL_ASSERT(parent != nullptr);

	Ref< EntityAdapter > resolvedEntityAdapter = new EntityAdapter(m_context);
	resolvedEntityAdapter->prepare(resolvedEntityData, nullptr, 0);

	parent->addChild(resolvedEntityAdapter);
	parent->swapChildren(externalAdapter, resolvedEntityAdapter);
	parent->removeChild(externalAdapter);

	updateScene();
	createInstanceGrid();
	return true;
}

bool SceneEditorPage::moveToEntity()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		m_context->moveToEntityAdapter(selectedEntities[0]);
	else
		m_context->moveToEntityAdapter(nullptr);
	return true;
}

bool SceneEditorPage::moveUp()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	EntityAdapter* moving = selectedEntities.back();
	T_ASSERT(moving != nullptr);

	EntityAdapter* parent = moving->getParent();
	if (!parent)
		return false;

	auto& children = parent->getChildren();
	auto it = std::find(children.begin(), children.end(), moving);
	if (it == children.begin() || it == children.end())
		return false;

	EntityAdapter* previousSibling = *(it - 1);
	T_ASSERT(previousSibling != nullptr);

	parent->swapChildren(moving, previousSibling);
	return true;
}

bool SceneEditorPage::moveDown()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	EntityAdapter* moving = selectedEntities.back();
	T_ASSERT(moving != nullptr);

	EntityAdapter* parent = moving->getParent();
	if (!parent)
		return false;

	auto& children = parent->getChildren();
	auto it = std::find(children.begin(), children.end(), moving);
	if (it == children.end() || it + 1 == children.end())
		return false;

	EntityAdapter* nextSibling = *(it + 1);
	T_ASSERT(nextSibling != nullptr);

	parent->swapChildren(moving, nextSibling);
	return true;
}

void SceneEditorPage::eventEntityToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void SceneEditorPage::eventGuideClick(ui::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::GridRow* row = event->getRow();
		std::wstring id = row->get(0)->getText();

		bool shouldDraw = !m_context->shouldDrawGuide(id);
		m_context->setDrawGuide(id, shouldDraw);

		row->set(1, new ui::GridItem(shouldDraw ? m_imageVisible : m_imageHidden));
		m_gridGuides->requestUpdate();

		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"SceneEditor.Guides/" + id, shouldDraw);
		m_editor->commitGlobalSettings();
	}
}

void SceneEditorPage::eventInstanceSelect(ui::SelectionChangeEvent* event)
{
	// De-select all entities.
	m_context->selectAllEntities(false);

	// Select only entities which is selected in the grid.
	RefArray< ui::GridRow > selectedRows;
	m_instanceGrid->getRows(selectedRows, ui::GridView::GfDescendants | ui::GridView::GfSelectedOnly);
	for (auto selectedRow : selectedRows)
	{
		EntityAdapter* entityAdapter = selectedRow->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

		m_context->selectEntity(entityAdapter);
	}

	// Raise context select event.
	m_context->raiseSelect();
}

void SceneEditorPage::eventInstanceExpand(ui::GridRowStateChangeEvent* event)
{
	ui::GridRow* row = event->getRow();
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	entityAdapter->setExpanded((row->getState() & ui::GridRow::RsExpanded) != 0);
}

void SceneEditorPage::eventInstanceButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		const ui::MenuItem* selectedItem;

		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.size() == 1)
		{
			if (selectedEntities[0]->isExternal())
				selectedItem = m_entityMenuExternal->showModal(m_instanceGrid, event->getPosition());
			else if (selectedEntities[0]->isGroup())
				selectedItem = m_entityMenuGroup->showModal(m_instanceGrid, event->getPosition());
			else
				selectedItem = m_entityMenuDefault->showModal(m_instanceGrid, event->getPosition());
		}

		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				event->consume();
		}
	}
}

void SceneEditorPage::eventInstanceClick(ui::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::GridRow* row = event->getRow();
		ui::GridItem* item = row->get(1);

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

		if (entityAdapter->isVisible(false))
		{
			item->setImage(m_imageHidden);
			entityAdapter->setVisible(false);
		}
		else
		{
			item->setImage(m_imageVisible);
			entityAdapter->setVisible(true);
		}

		m_instanceGrid->update();
	}
	else if (event->getColumn() == 2)
	{
		ui::GridRow* row = event->getRow();
		ui::GridItem* item = row->get(2);

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

		if (entityAdapter->isLocked())
		{
			item->setImage(m_imageUnlocked);
			entityAdapter->setLocked(false);
		}
		else
		{
			item->setImage(m_imageLocked);
			entityAdapter->setLocked(true);
		}

		m_instanceGrid->update();
	}
}

void SceneEditorPage::eventInstanceRename(ui::GridItemContentChangeEvent* event)
{
	std::wstring renameFrom = event->getOriginalText();
	std::wstring renameTo = event->getItem()->getText();

	if (renameFrom == renameTo)
		return;

	ui::GridItem* item = event->getItem();
	EntityAdapter* entityAdapter = item->getRow()->getData< EntityAdapter >(L"ENTITY");
	T_ASSERT(entityAdapter);

	m_context->getDocument()->push();

	entityAdapter->getEntityData()->setName(renameTo);
	updatePropertyObject();
	updateStatusBar();

	event->consume();
}

void SceneEditorPage::eventContextPostBuild(PostBuildEvent* event)
{
	createInstanceGrid();
	updateStatusBar();
}

void SceneEditorPage::eventContextSelect(ui::SelectionChangeEvent* event)
{
	updateInstanceGrid();
	updatePropertyObject();
	updateStatusBar();
}

void SceneEditorPage::eventContextPreModify(PreModifyEvent* event)
{
	m_context->getDocument()->push();
}

void SceneEditorPage::eventContextPostModify(PostModifyEvent* event)
{
	updatePropertyObject();
}

void SceneEditorPage::eventContextCameraMoved(CameraMovedEvent* event)
{
	updateStatusBar();
}

void SceneEditorPage::eventContextPostFrame(PostFrameEvent* event)
{
	updateStatusBar();
}

	}
}
