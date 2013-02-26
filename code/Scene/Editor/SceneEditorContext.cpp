#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepClone.h"
#include "Resource/IResourceManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/LayerEntityEditor.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Event.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntitySchema.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorContext", SceneEditorContext, ui::EventSubject)

SceneEditorContext::SceneEditorContext(
	editor::IEditor* editor,
	editor::IDocument* document,
	db::Database* resourceDb,
	db::Database* sourceDb,
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager
)
:	m_editor(editor)
,	m_document(document)
,	m_resourceDb(resourceDb)
,	m_sourceDb(sourceDb)
,	m_eventManager(eventManager)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_guideSize(2.0f)
,	m_pickEnable(true)
,	m_snapMode(SmNone)
,	m_snapSpacing(0.0f)
,	m_physicsEnable(false)
,	m_playing(false)
,	m_timeScale(1.0f)
,	m_time(0.0f)
{
	for (int i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = new Camera();
}

SceneEditorContext::~SceneEditorContext()
{
	destroy();
}

void SceneEditorContext::destroy()
{
	m_editor = 0;
	m_document = 0;
	m_resourceDb = 0;
	m_sourceDb = 0;
	m_resourceManager = 0;
	m_renderSystem = 0;
	for (int32_t i = 0; i < sizeof_array(m_debugTexture); ++i)
		m_debugTexture[i] = 0;
	m_physicsManager = 0;
	m_editorProfiles.clear();
	m_editorPlugins.clear();
	m_controllerEditor = 0;
	m_modifier = 0;
	for (int32_t i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = 0;
	m_sceneAsset = 0;
	m_scene = 0;
	m_layerEntityAdapters.clear();
	m_followEntityAdapter = 0;
}

void SceneEditorContext::addEditorProfile(ISceneEditorProfile* editorProfile)
{
	m_editorProfiles.push_back(editorProfile);
}

void SceneEditorContext::addEditorPlugin(ISceneEditorPlugin* editorPlugin)
{
	m_editorPlugins.push_back(editorPlugin);
}

void SceneEditorContext::setControllerEditor(ISceneControllerEditor* controllerEditor)
{
	m_controllerEditor = controllerEditor;
}

void SceneEditorContext::setModifier(IModifier* modifier)
{
	if ((m_modifier = modifier) != 0)
		m_modifier->selectionChanged();
}

IModifier* SceneEditorContext::getModifier() const
{
	return m_modifier;
}

void SceneEditorContext::setGuideSize(float guideSize)
{
	m_guideSize = guideSize;
}

float SceneEditorContext::getGuideSize() const
{
	return m_guideSize;
}

void SceneEditorContext::setPickEnable(bool pickEnable)
{
	m_pickEnable = pickEnable;
}

bool SceneEditorContext::getPickEnable() const
{
	return m_pickEnable;
}

void SceneEditorContext::setSnapMode(SnapMode snapMode)
{
	if (m_snapMode != snapMode)
	{
		m_snapMode = snapMode;
		if (m_modifier)
			m_modifier->selectionChanged();
	}
}

SceneEditorContext::SnapMode SceneEditorContext::getSnapMode() const
{
	return m_snapMode;
}

void SceneEditorContext::setSnapSpacing(float snapSpacing)
{
	if (m_snapSpacing != snapSpacing)
	{
		m_snapSpacing = snapSpacing;
		if (m_modifier)
			m_modifier->selectionChanged();
	}
}

float SceneEditorContext::getSnapSpacing() const
{
	return m_snapSpacing;
}

void SceneEditorContext::setPhysicsEnable(bool physicsEnable)
{
	m_physicsEnable = physicsEnable;
}

bool SceneEditorContext::getPhysicsEnable() const
{
	return m_physicsEnable;
}

Camera* SceneEditorContext::getCamera(int index) const
{
	T_ASSERT (index >= 0)
	T_ASSERT (index < sizeof_array(m_cameras));
	return m_cameras[index];
}

void SceneEditorContext::setFollowEntityAdapter(EntityAdapter* followEntityAdapter)
{
	m_followEntityAdapter = followEntityAdapter;
}

void SceneEditorContext::setLookAtEntityAdapter(EntityAdapter* lookAtEntityAdapter)
{
	m_lookAtEntityAdapter = lookAtEntityAdapter;
}

void SceneEditorContext::moveToEntityAdapter(EntityAdapter* entityAdapter)
{
	if (!entityAdapter)
		return;

	Aabb3 boundingBox = entityAdapter->getBoundingBox();
	if (boundingBox.empty())
		return;

	Scalar distance = boundingBox.getExtent().get(majorAxis3(boundingBox.getExtent())) * Scalar(3.0f);
	
	Transform T = entityAdapter->getTransform();

	for (uint32_t i = 0; i < sizeof_array(m_cameras); ++i)
	{
		T_ASSERT (m_cameras[i]);

		Vector4 P = T.translation() - m_cameras[i]->getOrientation() * Vector4(0.0f, 0.0f, distance, 0.0f);
		m_cameras[i]->place(P);
	}
}

void SceneEditorContext::setPlaying(bool playing)
{
	m_playing = playing;
}

bool SceneEditorContext::isPlaying() const
{
	return m_playing;
}

void SceneEditorContext::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float SceneEditorContext::getTimeScale() const
{
	return m_timeScale;
}

void SceneEditorContext::setTime(float time)
{
	m_time = time;
}

float SceneEditorContext::getTime() const
{
	return m_time;
}

void SceneEditorContext::drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter)
{
	if (entityAdapter)
	{
		IEntityEditor* entityEditor = entityAdapter->getEntityEditor();
		if (entityEditor)
			entityEditor->drawGuide(primitiveRenderer);
	}
}

void SceneEditorContext::setSceneAsset(SceneAsset* sceneAsset)
{
	m_sceneAsset = sceneAsset;
}

void SceneEditorContext::buildEntities()
{
	m_scene = 0;

	if (m_sceneAsset)
	{
		// Create entity editor factories.
		RefArray< const IEntityEditorFactory > entityEditorFactories;
		for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
			(*i)->createEntityEditorFactories(this, entityEditorFactories);

		Ref< world::IEntitySchema > entitySchema = new world::EntitySchema();
		Ref< world::EntityBuilder > entityBuilder = new world::EntityBuilder();
		Ref< world::EntityBuilderWithSchema > entityBuilderSchema = new world::EntityBuilderWithSchema(entityBuilder, entitySchema);
		Ref< EntityAdapterBuilder > entityAdapterBuilder = new EntityAdapterBuilder(this, entityBuilderSchema, entityEditorFactories);

		// Create entity factories.
		for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
		{
			RefArray< const world::IEntityFactory > entityFactories;
			(*i)->createEntityFactories(this, entityFactories);

			for (RefArray< const world::IEntityFactory >::iterator j = entityFactories.begin(); j != entityFactories.end(); ++j)
				entityAdapterBuilder->addFactory(*j);
		}

		// Create root group entity as scene instances doesn't have a concept of layers.
		Ref< world::GroupEntity > rootGroupEntity = new world::GroupEntity();

		// Create entities from scene layers.
		const RefArray< world::LayerEntityData >& layers = m_sceneAsset->getLayers();

		m_layerEntityAdapters.resize(layers.size());
		for (uint32_t i = 0; i < layers.size(); ++i)
		{
			world::LayerEntityData* layerEntityData = layers[i];
			T_ASSERT (layerEntityData);

			// If possible reuse layer entity adapter.
			if (!m_layerEntityAdapters[i])
			{
				m_layerEntityAdapters[i] = new EntityAdapter();

				// Copy initial state from data.
				m_layerEntityAdapters[i]->setVisible(layerEntityData->isVisible());
				m_layerEntityAdapters[i]->setLocked(layerEntityData->isLocked());
			}

			// Create a layer group entity.
			Ref< world::GroupEntity > layerEntity = new world::GroupEntity();
			m_layerEntityAdapters[i]->setEntityData(layerEntityData);
			m_layerEntityAdapters[i]->setEntity(layerEntity);
			m_layerEntityAdapters[i]->setEntityEditor(new LayerEntityEditor(layerEntityData));
		
			// Create layer's child entities.
			const RefArray< world::EntityData >& layerChildEntityData = layerEntityData->getEntityData();
			for (RefArray< world::EntityData >::const_iterator j = layerChildEntityData.begin(); j != layerChildEntityData.end(); ++j)
			{
				Ref< world::Entity > entity = entityAdapterBuilder->create(*j);
				if (!entity)
					continue;

				Ref< EntityAdapter > entityAdapter = entityAdapterBuilder->getRootAdapter();
				T_ASSERT (entityAdapter->getEntity() == entity);

				layerEntity->addEntity(entity);
				m_layerEntityAdapters[i]->link(entityAdapter);
			}

			// Add layer to root entity.
			rootGroupEntity->addEntity(layerEntity);
		}

		// Update scene controller also.
		Ref< ISceneController > controller;
		if (m_sceneAsset->getControllerData())
		{
			RefArray< EntityAdapter > entityAdapters;
			getEntities(entityAdapters);

			std::map< const world::EntityData*, Ref< world::Entity > > entityProducts;
			for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
				entityProducts.insert(std::make_pair(
					(*i)->getEntityData(),
					(*i)->getEntity()
				));

			controller = m_sceneAsset->getControllerData()->createController(entityProducts);
		}

		T_DEBUG(entityAdapterBuilder->getAdapterCount() << L" entity adapter(s) built");

		// Bind post process settings.
		resource::Proxy< world::PostProcessSettings > postProcessSettings;
		m_resourceManager->bind(m_sceneAsset->getPostProcessSettings(), postProcessSettings);

		// Create our scene.
		m_scene = new Scene(
			controller,
			entitySchema,
			rootGroupEntity,
			m_sceneAsset->getWorldRenderSettings(),
			postProcessSettings
		);
	}

	// Create map from entity to adapter.
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	m_entityAdapterMap.clear();
	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		m_entityAdapterMap.insert((*i)->getEntity(), *i);

	raisePostBuild();
}

void SceneEditorContext::selectEntity(EntityAdapter* entityAdapter, bool select)
{
	if (entityAdapter && entityAdapter->m_selected != select)
	{
		entityAdapter->m_selected = select;
		if (entityAdapter->m_entityEditor)
			entityAdapter->m_entityEditor->entitySelected(select);
	}
}

void SceneEditorContext::selectAllEntities(bool select)
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		selectEntity(*i, select);
}

uint32_t SceneEditorContext::getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags) const
{
	typedef std::pair< RefArray< EntityAdapter >::const_iterator, RefArray< EntityAdapter >::const_iterator > range_t;

	outEntityAdapters.resize(0);

	if (m_layerEntityAdapters.empty())
		return 0;

	std::stack< range_t > stack;
	RefArray< EntityAdapter > rootEntityAdapters = m_layerEntityAdapters;

	stack.push(std::make_pair(rootEntityAdapters.begin(), rootEntityAdapters.end()));
	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			EntityAdapter* entityAdapter = *r.first++;

			bool include = true;

			if (flags & GfSelectedOnly)
				include &= entityAdapter->isSelected();
			if (flags & GfNoSelected)
				include &= !entityAdapter->isSelected();

			if (flags & GfExternalOnly)
				include &= entityAdapter->isExternal();
			if (flags & GfNoExternal)
				include &= !entityAdapter->isExternal();

			if (flags & GfExternalChildOnly)
				include &= entityAdapter->isChildOfExternal();
			if (flags & GfNoExternalChild)
				include &= !entityAdapter->isChildOfExternal();

			if (include)
				outEntityAdapters.push_back(entityAdapter);

			if (flags & GfDescendants)
			{
				const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
				if (!children.empty())
					stack.push(std::make_pair(children.begin(), children.end()));
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outEntityAdapters.size());
}

EntityAdapter* SceneEditorContext::findAdapterFromEntity(const world::Entity* entity) const
{
	SmallMap< const world::Entity*, EntityAdapter* >::const_iterator i = m_entityAdapterMap.find(entity);
	return i != m_entityAdapterMap.end() ? i->second : 0;
}

EntityAdapter* SceneEditorContext::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, bool onlyPickable) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	EntityAdapter* minEntityAdapter = 0;
	Scalar minDistance(1e8f);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		// Must be public, unlocked, visible and no child of external.
		if (
			(*i)->isPrivate() ||
			(*i)->isLocked() ||
			!(*i)->isVisible() ||
			(*i)->isChildOfExternal()
		)
			continue;

		IEntityEditor* entityEditor = (*i)->getEntityEditor();
		if (!entityEditor)
			continue;

		// Do not trace non-pickable.
		if (onlyPickable && !entityEditor->isPickable())
			continue;

		// Trace bounding box to see if ray intersect.
		Scalar distance = minDistance;
		if (entityEditor->queryRay(worldRayOrigin, worldRayDirection, distance))
		{
			if (distance < minDistance)
			{
				minEntityAdapter = *i;
				minDistance = distance;
			}
		}
	}

	return minEntityAdapter;
}

uint32_t SceneEditorContext::queryFrustum(const Frustum& worldFrustum, RefArray< EntityAdapter >& outEntityAdapters, bool onlyPickable) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		// Must be public, unlocked, visible and no child of external.
		if (
			(*i)->isPrivate() ||
			(*i)->isLocked() ||
			!(*i)->isVisible() ||
			(*i)->isChildOfExternal()
		)
			continue;

		IEntityEditor* entityEditor = (*i)->getEntityEditor();
		if (!entityEditor)
			continue;

		// Do not trace non-pickable.
		if (onlyPickable && !entityEditor->isPickable())
			continue;

		// Query if entity inside frustum.
		if (entityEditor->queryFrustum(worldFrustum))
			outEntityAdapters.push_back(*i);
	}

	return outEntityAdapters.size();
}

void SceneEditorContext::cloneSelected()
{
	RefArray< EntityAdapter > selectedEntityAdapters;
	getEntities(selectedEntityAdapters, GfDescendants | GfSelectedOnly);
	if (selectedEntityAdapters.empty())
		return;

	for (RefArray< EntityAdapter >::iterator i = selectedEntityAdapters.begin(); i != selectedEntityAdapters.end(); ++i)
	{
		Ref< EntityAdapter > parentContainerGroup = (*i)->getParentContainerGroup();
		if (!parentContainerGroup)
			continue;

		Ref< world::EntityData > clonedEntityData = DeepClone((*i)->getEntityData()).create< world::EntityData >();
		T_ASSERT (clonedEntityData);

		Ref< EntityAdapter > clonedEntityAdapter = new EntityAdapter();
		clonedEntityAdapter->setEntityData(clonedEntityData);
		parentContainerGroup->addChild(clonedEntityAdapter);

		(*i)->m_selected = false;
		clonedEntityAdapter->m_selected = true;
	}

	buildEntities();
	raiseSelect(this);
}

void SceneEditorContext::setDebugTexture(uint32_t index, render::ITexture* debugTexture)
{
	m_debugTexture[index] = debugTexture;
}

render::ITexture* SceneEditorContext::getDebugTexture(uint32_t index)
{
	return m_debugTexture[index];
}

ISceneEditorPlugin* SceneEditorContext::getEditorPluginOf(const TypeInfo& pluginType) const
{
	for (RefArray< ISceneEditorPlugin >::const_iterator i = m_editorPlugins.begin(); i != m_editorPlugins.end(); ++i)
	{
		if (&type_of(*i) == &pluginType)
			return *i;
	}
	return 0;
}

void SceneEditorContext::raisePreModify()
{
	raiseEvent(EiPreModify, 0);
}

void SceneEditorContext::raisePostModify()
{
	raiseEvent(EiPostModify, 0);
}

void SceneEditorContext::raisePostFrame(ui::Event* event)
{
	raiseEvent(EiPostFrame, event);
}

void SceneEditorContext::raisePostBuild()
{
	raiseEvent(EiPostBuild, 0);
}

void SceneEditorContext::raiseSelect(Object* item)
{
	// Notify modifier about selection change.
	if (m_modifier)
		m_modifier->selectionChanged();

	// Notify selection change event listeners.
	ui::Event event(this, item);
	raiseEvent(EiSelect, &event);
}

void SceneEditorContext::raiseCameraMoved()
{
	raiseEvent(EiCameraMoved, 0);
}

void SceneEditorContext::addPreModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPreModify, eventHandler);
}

void SceneEditorContext::addPostModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostModify, eventHandler);
}

void SceneEditorContext::addPostFrameEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostFrame, eventHandler);
}

void SceneEditorContext::addPostBuildEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostBuild, eventHandler);
}

void SceneEditorContext::addSelectEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiSelect, eventHandler);
}

void SceneEditorContext::addCameraMovedEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiCameraMoved, eventHandler);
}

	}
}
