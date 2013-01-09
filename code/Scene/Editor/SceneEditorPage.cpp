#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
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
#include "Scene/Editor/SelectEvent.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"

// Resources
#include "Resources/EntityEdit.h"
#include "Resources/EntityTypes.h"
#include "Resources/LayerHidden.h"
#include "Resources/LayerVisible.h"
#include "Resources/LayerLocked.h"
#include "Resources/LayerUnlocked.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

bool isChildEntitySelected(const EntityAdapter* entityAdapter)
{
	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if ((*i)->isSelected())
			return true;

		if (isChildEntitySelected(*i))
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
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	// Get physics manager type.
	std::wstring physicsManagerTypeName = m_editor->getSettings()->getProperty< PropertyString >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName);
	if (!physicsManagerType)
	{
		log::error << L"Unable to create scene editor; no such physics manager type \"" << physicsManagerTypeName << L"\"." << Endl;
		return false;
	}

	// Create physics manager.
	Ref< physics::PhysicsManager > physicsManager = checked_type_cast< physics::PhysicsManager* >(physicsManagerType->createInstance());
	if (!physicsManager->create(1.0f / 60.0f))
	{
		log::error << L"Unable to create scene editor; failed to create physics manager." << Endl;
		return false;
	}

	// Configure physics manager.
	physicsManager->setGravity(Vector4(0.0f, -9.81f, 0.0f, 0.0f));

	// Create resource manager.
	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(true);

	// Create editor context.
	m_context = new SceneEditorContext(
		m_editor,
		m_document,
		m_editor->getOutputDatabase(),
		m_editor->getSourceDatabase(),
		resourceManager,
		renderSystem,
		physicsManager
	);

	// Create profiles, plugins, resource factories and entity editors.
	std::vector< const TypeInfo* > profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (std::vector< const TypeInfo* >::const_iterator i = profileTypes.begin(); i != profileTypes.end(); ++i)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >((*i)->createInstance());
		if (!profile)
			continue;

		m_context->addEditorProfile(profile);

		RefArray< ISceneEditorPlugin > editorPlugins;
		profile->createEditorPlugins(m_context, editorPlugins);
		for (RefArray< ISceneEditorPlugin >::iterator j = editorPlugins.begin(); j != editorPlugins.end(); ++j)
			m_context->addEditorPlugin(*j);

		RefArray< resource::IResourceFactory > resourceFactories;
		profile->createResourceFactories(m_context, resourceFactories);
		for (RefArray< resource::IResourceFactory >::iterator j = resourceFactories.begin(); j != resourceFactories.end(); ++j)
			resourceManager->addFactory(*j);
	}

	// Create editor panel.
	m_editPanel = new ui::Container();
	m_editPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0));

	m_editControl = new ScenePreviewControl();
	m_editControl->create(m_editPanel, m_context);

	m_statusBar = new ui::custom::StatusBar();
	m_statusBar->create(m_editPanel);

	// Create entity panel.
	m_entityPanel = new ui::Container();
	m_entityPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenu = new ui::PopupMenu();
	m_entityMenu->create();
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_toolLookAtEntity = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_LOOK_AT_ENTITY"), ui::Command(L"Scene.Editor.LookAtEntity"), 3, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolFollowEntity = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_FOLLOW_ENTITY"), ui::Command(L"Scene.Editor.FollowEntity"), 4, ui::custom::ToolBarButton::BsDefaultToggle);

	m_entityToolBar = new ui::custom::ToolBar();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(ui::Bitmap::load(c_ResourceEntityEdit, sizeof(c_ResourceEntityEdit), L"png"), 5);
	m_entityToolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), ui::Command(L"Editor.Delete"), 2));
	m_entityToolBar->addItem(new ui::custom::ToolBarSeparator());
	m_entityToolBar->addItem(m_toolLookAtEntity);
	m_entityToolBar->addItem(m_toolFollowEntity);
	m_entityToolBar->addItem(new ui::custom::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY"), ui::Command(L"Scene.Editor.MoveToEntity"), 1));
	m_entityToolBar->addClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityToolClick));

	m_imageHidden = ui::Bitmap::load(c_ResourceLayerHidden, sizeof(c_ResourceLayerHidden), L"png");
	m_imageVisible = ui::Bitmap::load(c_ResourceLayerVisible, sizeof(c_ResourceLayerVisible), L"png");
	m_imageLocked = ui::Bitmap::load(c_ResourceLayerLocked, sizeof(c_ResourceLayerLocked), L"png");
	m_imageUnlocked = ui::Bitmap::load(c_ResourceLayerUnlocked, sizeof(c_ResourceLayerUnlocked), L"png");

	m_instanceGrid = new ui::custom::GridView();
	m_instanceGrid->create(m_entityPanel, ui::WsDoubleBuffer);
	//m_instanceGrid->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_instanceGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCENE_EDITOR_ENTITY_NAME"), 200));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", 30));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", 30));
	m_instanceGrid->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceSelect));
	m_instanceGrid->addExpandEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceExpand));
	m_instanceGrid->addButtonDownEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceButtonDown));
	m_instanceGrid->addClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceClick));

	m_instanceGridFontBold = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontBold->setBold(true);

	m_instanceGridFontHuge = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontHuge->setSize(12);

	m_site->createAdditionalPanel(m_entityPanel, 300, false);

	// Create dependency panel.
	m_entityDependencyPanel = new EntityDependencyInvestigator(m_context);
	m_entityDependencyPanel->create(parent);

	m_site->createAdditionalPanel(m_entityDependencyPanel, 300, false);

	// Create controller panel.
	m_controllerPanel = new ui::Container();
	m_controllerPanel->create(parent, ui::WsNone, new ui::FloodLayout());
	m_controllerPanel->setText(i18n::Text(L"SCENE_EDITOR_CONTROLLER"));

	m_site->createAdditionalPanel(m_controllerPanel, 120, true);

	// Context event handlers.
	m_context->addPostBuildEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPostBuild));
	m_context->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextSelect));
	m_context->addPreModifyEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPreModify));
	m_context->addPostModifyEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPostModify));
	m_context->addCameraMovedEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextCameraMoved));

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
	// Destroy controller editor.
	if (m_context->getControllerEditor())
		m_context->getControllerEditor()->destroy();

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_entityPanel);
	m_site->destroyAdditionalPanel(m_entityDependencyPanel);
	m_site->destroyAdditionalPanel(m_controllerPanel);

	// Destroy widgets.
	safeDestroy(m_editPanel);
	safeDestroy(m_editControl);
	safeDestroy(m_entityPanel);
	safeDestroy(m_entityDependencyPanel);
	safeDestroy(m_entityMenu);
	safeDestroy(m_controllerPanel);
	safeDestroy(m_entityToolBar);
	safeDestroy(m_instanceGrid);

	m_toolLookAtEntity = 0;
	m_toolFollowEntity = 0;

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();

	m_context->destroy();
	m_context = 0;
}

void SceneEditorPage::activate()
{
	m_editControl->setVisible(true);
}

void SceneEditorPage::deactivate()
{
	m_editControl->setVisible(false);
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	// Get index of view where user dropped instance.
	uint32_t viewIndex;
	if (!m_editControl->getViewIndex(position, viewIndex))
		return false;

	Ref< world::EntityData > entityData;

	// Check profiles if any can convert instance into an entity data.
	const RefArray< ISceneEditorProfile >& editorProfiles = m_context->getEditorProfiles();
	for (RefArray< ISceneEditorProfile >::const_iterator i = editorProfiles.begin(); i != editorProfiles.end(); ++i)
	{
		if ((entityData = (*i)->createEntityData(m_context, instance)) != 0)
			break;
	}

	if (entityData)
	{
		Ref< EntityAdapter > parentGroupAdapter;

		// Get selected items, must be a single item.
		RefArray< ui::custom::GridRow > selectedRows;
		if (m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly) == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT (selectedEntity);

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
		T_ASSERT (documentObject);

		if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(documentObject))
		{
			if (parentGroupAdapter->isLayer())
			{
				log::error << L"Unable to drop entity; no prefab group selected" << Endl;
				return false;
			}
		}

		m_context->getDocument()->push();

		// Create instance and adapter.
		Ref< EntityAdapter > entityAdapter = new EntityAdapter();
		entityAdapter->setEntityData(entityData);

		// Place instance in front of perspective camera.
		const Camera* camera = m_context->getCamera(viewIndex);
		T_ASSERT (camera);

		Matrix44 Mworld = camera->getWorld() * translate(0.0f, 0.0f, 4.0f);
		entityAdapter->setTransform(Transform(Mworld.translation()));

		// Finally add adapter to parent group.
		parentGroupAdapter->addChild(entityAdapter);

		updateScene();
		createInstanceGrid();

		// Select entity.
		m_context->selectAllEntities(false);
		m_context->selectEntity(entityAdapter);
		m_context->raiseSelect(this);
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

		m_context->raiseSelect(this);
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_context->getDocument()->undo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect(this);
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_context->getDocument()->redo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect(this);
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

		// Create clipboard data with all selected entities; remove entities from scene if we're cutting.
		Ref< EntityClipboardData > entityClipboardData = new EntityClipboardData();
		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			entityClipboardData->addEntityData((*i)->getEntityData());
			if (command == L"Editor.Cut")
			{
				Ref< EntityAdapter > parentGroup = (*i)->getParent();
				if (parentGroup->isGroup())
				{
					parentGroup->removeChild(*i);

					if (m_context->getControllerEditor())
						m_context->getControllerEditor()->entityRemoved(*i);
				}
			}
		}

		ui::Application::getInstance()->getClipboard()->setObject(entityClipboardData);

		if (command == L"Editor.Cut")
		{
			updateScene();
			createInstanceGrid();

			m_context->raiseSelect(this);
		}
	}
	else if (command == L"Editor.Paste")
	{
		// Get parent group under which we will place the new entity.
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
			return false;

		Ref< EntityAdapter > parentEntity = selectedEntities[0]->getParentGroup();
		T_ASSERT (parentEntity);

		// Get clipboard data; ensure correct type.
		Ref< EntityClipboardData > entityClipboardData = dynamic_type_cast< EntityClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!entityClipboardData)
			return false;

		const RefArray< world::EntityData >& entityData = entityClipboardData->getEntityData();
		if (entityData.empty())
			return false;

		m_context->getDocument()->push();

		// Create new instances and adapters for each entity found in clipboard.
		for (RefArray< world::EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< EntityAdapter > entityAdapter = new EntityAdapter();
			entityAdapter->setEntityData(*i);
			parentEntity->addChild(entityAdapter);
		}

		updateScene();
		createInstanceGrid();

		m_context->raiseSelect(this);
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

		uint32_t removedCount = 0;
		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			Ref< EntityAdapter > parentGroup = (*i)->getParent();
			if (parentGroup && parentGroup->isGroup())
			{
				parentGroup->removeChild(*i);
				removedCount++;

				if (m_context->getControllerEditor())
					m_context->getControllerEditor()->entityRemoved(*i);
			}
		}

		if (removedCount)
		{
			updateScene();
			createInstanceGrid();

			m_context->raiseSelect(this);
		}
	}
	else if (command == L"Editor.SelectAll")
	{
		m_context->selectAllEntities();
		m_context->raiseSelect(this);
	}
	else if (command == L"Scene.Editor.AddEntity")
		result = addEntity();
	else if (command == L"Scene.Editor.LookAtEntity")
		result = updateLookAtEntity();
	else if (command == L"Scene.Editor.FollowEntity")
		result = updateFollowEntity();
	else if (command == L"Scene.Editor.MoveToEntity")
		result = moveToEntity();
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
		m_context->setGuideSize(2.0f);
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

void SceneEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (!m_context)
		return;

	m_context->getResourceManager()->reload(eventId);

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	bool externalModified = false;
	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Guid externalGuid;
		if ((*i)->getExternalGuid(externalGuid))
		{
			if (externalGuid == eventId)
			{
				// Modified external entity detected; need to recreate the scene.
				externalModified = true;
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

	if (SceneAsset* sceneAsset = dynamic_type_cast< SceneAsset* >(documentObject))
		m_context->setSceneAsset(sceneAsset);
	else if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(documentObject))
	{
		Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
		if (!sceneAsset)
			return false;

		const RefArray< world::LayerEntityData >& layers = sceneAsset->getLayers();
		T_ASSERT (layers.size() >= 2);

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
		m_context->setControllerEditor(0);
	}

	Ref< SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		Ref< ISceneControllerData > controllerData = sceneAsset->getControllerData();
		if (controllerData)
		{
			RefArray< ISceneControllerEditorFactory > controllerEditorFactories;
			Ref< ISceneControllerEditor > controllerEditor;

			// Create controller editor factories.
			const RefArray< ISceneEditorProfile >& profiles = m_context->getEditorProfiles();
			for (RefArray< ISceneEditorProfile >::const_iterator i = profiles.begin(); i != profiles.end(); ++i)
				(*i)->createControllerEditorFactories(m_context, controllerEditorFactories);

			for (RefArray< ISceneControllerEditorFactory >::iterator i = controllerEditorFactories.begin(); i != controllerEditorFactories.end(); ++i)
			{
				TypeInfoSet typeSet = (*i)->getControllerDataTypes();
				if (typeSet.find(&type_of(controllerData)) != typeSet.end())
				{
					controllerEditor = (*i)->createControllerEditor(type_of(controllerData));
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
					m_controllerPanel->update();
				}
				else
					log::error << L"Unable to create controller editor; create failed" << Endl;
			}
			else
				T_DEBUG(L"Unable to find controller editor for type \"" << type_name(controllerData) << L"\"");
		}
	}
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		// Check if any scene settings has changed.
		bool needUpdate = false;

		if (m_currentGuid != sceneAsset->getPostProcessSettings())
		{
			needUpdate = true;
			m_currentGuid = sceneAsset->getPostProcessSettings();
		}

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

Ref< ui::custom::GridRow > SceneEditorPage::createInstanceGridRow(EntityAdapter* entityAdapter)
{
	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(
		(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::custom::GridRow::RsExpanded : 0)
	);

	std::wstring entityName = entityAdapter->getName();
	if (entityName.empty())
		entityName = i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY");

	if (entityAdapter->isExternal())
		row->add(new ui::custom::GridItem(entityName, m_instanceGridFontBold/*, 1*/));
	else if (entityAdapter->isLayer())
	{
		row->add(new ui::custom::GridItem(entityName, m_instanceGridFontHuge/*, 4*/));

		bool childSelected = isChildEntitySelected(entityAdapter);
		row->setBackground(
			childSelected ?
				Color4ub(180, 190, 240, 255) :
				Color4ub(220, 220, 230, 255)
		);

		row->setMinimumHeight(32);
	}
	else if (entityAdapter->isGroup())
	{
		row->add(new ui::custom::GridItem(entityName/*, 2, 3*/));

		bool childSelected = isChildEntitySelected(entityAdapter);
		row->setBackground(
			childSelected ?
				Color4ub(180, 190, 240, 255) :
				Color4ub(255, 255, 255, 255)
		);
	}
	else
		row->add(new ui::custom::GridItem(entityName/*, 0*/));

	// Create "visible" check box.
	row->add(new ui::custom::GridItem(
		entityAdapter->isVisible(false) ? m_imageVisible : m_imageHidden
	));

	// Create "locked" check box.
	row->add(new ui::custom::GridItem(
		entityAdapter->isLocked(false) ? m_imageLocked : m_imageUnlocked
	));

	// Recursively add children.
	if (
		!entityAdapter->isExternal() &&
		!entityAdapter->isChildrenPrivate()
	)
	{
		const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
		for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			Ref< ui::custom::GridRow > child = createInstanceGridRow(*i);
			if (child)
				row->addChild(child);
		}
	}

	return row;
}

void SceneEditorPage::createInstanceGrid()
{
	m_instanceGrid->removeAllRows();

	const RefArray< EntityAdapter >& layerEntityAdapters = m_context->getLayerEntityAdapters();
	for (RefArray< EntityAdapter >::const_iterator j = layerEntityAdapters.begin(); j != layerEntityAdapters.end(); ++j)
	{
		Ref< ui::custom::GridRow > entityRow = createInstanceGridRow(*j);
		if (entityRow)
			m_instanceGrid->addRow(entityRow);
	}

	m_instanceGrid->update();
}

void SceneEditorPage::updateInstanceGridRow(ui::custom::GridRow* row)
{
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");

	bool childSelected = isChildEntitySelected(entityAdapter);
	if (entityAdapter->isLayer())
	{
		row->setBackground(
			childSelected ?
				Color4ub(180, 190, 240, 255) :
				Color4ub(220, 220, 230, 255)
		);
	}
	else
	{
		row->setBackground(
			childSelected ?
				Color4ub(180, 190, 240, 255) :
				Color4ub(255, 255, 255, 255)
		);
	}

	row->setState(
		(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::custom::GridRow::RsExpanded : 0)
	);

	const RefArray< ui::custom::GridRow >& childRows = row->getChildren();
	for (RefArray< ui::custom::GridRow >::const_iterator i = childRows.begin(); i != childRows.end(); ++i)
		updateInstanceGridRow(*i);
}

void SceneEditorPage::updateInstanceGrid()
{
	const RefArray< ui::custom::GridRow >& rows = m_instanceGrid->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
		updateInstanceGridRow(*i);

	m_instanceGrid->update();
}

void SceneEditorPage::updatePropertyObject()
{
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly);

	if (entityAdapters.size() == 1)
	{
		Ref< EntityAdapter > entityAdapter = entityAdapters.front();
		T_ASSERT (entityAdapter);

		m_site->setPropertyObject(entityAdapter->getEntityData());
	}
	else
		m_site->setPropertyObject(m_context->getDocument()->getObject(0));
}

void SceneEditorPage::updateStatusBar()
{
	const Camera* camera = m_context->getCamera(0);
	T_ASSERT (camera);

	Vector4 position = camera->getPosition();
	Vector4 angles = camera->getOrientation().toEulerAngles();

	StringOutputStream ss;
	ss.setDecimals(2);
	ss << position.x() << L", " << position.y() << L", " << position.z() << L"     ";
	ss << rad2deg(angles.x()) << L"\176, " << rad2deg(angles.y()) << L"\176, " << rad2deg(angles.z()) << L"\176";

	m_statusBar->setText(ss.str());
}

bool SceneEditorPage::addEntity()
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
	const TypeInfo* entityType = m_context->getEditor()->browseType(&type_of< world::EntityData >());
	if (!entityType)
		return false;

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->createInstance());
	T_ASSERT (entityData);

	m_context->getDocument()->push();

	Ref< EntityAdapter > entityAdapter = new EntityAdapter();
	entityAdapter->setEntityData(entityData);
	parentGroupAdapter->addChild(entityAdapter);

	updateScene();
	createInstanceGrid();

	return true;
}

bool SceneEditorPage::updateLookAtEntity()
{
	if (m_toolLookAtEntity->isToggled())
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		{
			m_context->setLookAtEntityAdapter(selectedEntities[0]);
			return true;
		}
	}
	m_context->setLookAtEntityAdapter(0);
	return true;
}

bool SceneEditorPage::updateFollowEntity()
{
	if (m_toolFollowEntity->isToggled())
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		{
			m_context->setFollowEntityAdapter(selectedEntities[0]);
			return true;
		}
	}
	m_context->setFollowEntityAdapter(0);
	return true;
}

bool SceneEditorPage::moveToEntity()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		m_context->moveToEntityAdapter(selectedEntities[0]);
	else
		m_context->moveToEntityAdapter(0);
	return true;
}

void SceneEditorPage::eventEntityToolClick(ui::Event* event)
{
	ui::CommandEvent* commandEvent = checked_type_cast< ui::CommandEvent* >(event);
	handleCommand(commandEvent->getCommand());
}

void SceneEditorPage::eventInstanceSelect(ui::Event* event)
{
	// De-select all entities.
	m_context->selectAllEntities(false);

	// Select only entities which is selected in the grid.
	RefArray< ui::custom::GridRow > selectedRows;
	m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly);

	for (RefArray< ui::custom::GridRow >::iterator i = selectedRows.begin(); i != selectedRows.end(); ++i)
	{
		EntityAdapter* entityAdapter = (*i)->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		m_context->selectEntity(entityAdapter);
	}

	// Raise context select event.
	m_context->raiseSelect(this);
}

void SceneEditorPage::eventInstanceExpand(ui::Event* event)
{
	ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(event->getItem());
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	entityAdapter->setExpanded((row->getState() & ui::custom::GridRow::RsExpanded) != 0);
}

void SceneEditorPage::eventInstanceButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_entityMenu->show(m_instanceGrid, mouseEvent->getPosition());
		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				mouseEvent->consume();
		}
	}
}

void SceneEditorPage::eventInstanceClick(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

	if (cmd.getId() == 1)
	{
		ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(cmdEvent->getItem());
		ui::custom::GridItem* item = checked_type_cast< ui::custom::GridItem*, false >(row->get(1));

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

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
	else if (cmd.getId() == 2)
	{
		ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(cmdEvent->getItem());
		ui::custom::GridItem* item = checked_type_cast< ui::custom::GridItem*, false >(row->get(2));

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

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

void SceneEditorPage::eventContextPostBuild(ui::Event* event)
{
	createInstanceGrid();
}

void SceneEditorPage::eventContextSelect(ui::Event* event)
{
	updateInstanceGrid();
	updatePropertyObject();
}

void SceneEditorPage::eventContextPreModify(ui::Event* event)
{
	m_context->getDocument()->push();
}

void SceneEditorPage::eventContextPostModify(ui::Event* event)
{
	updatePropertyObject();
}

void SceneEditorPage::eventContextCameraMoved(ui::Event* event)
{
	updateStatusBar();
}

	}
}
