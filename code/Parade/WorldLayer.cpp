#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Parade/WorldLayer.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Scene/Scene.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/TransientEntity.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/IEntitySchema.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const Color4f c_clearColor(0.0f, 0.0f, 0.0f, 0.0f);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.WorldLayer", WorldLayer, Layer)

WorldLayer::WorldLayer(
	Stage* stage,
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< scene::Scene >& scene,
	const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
)
:	Layer(stage, name)
,	m_environment(environment)
,	m_scene(scene)
,	m_entities(entities)
,	m_dynamicEntities(new world::GroupEntity())
,	m_alternateTime(0.0f)
,	m_deltaTime(0.0f)
,	m_fieldOfView(70.0f)
,	m_controllerEnable(true)
{
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< PropertyFloat >(L"World.FieldOfView", 70.0f);
}

void WorldLayer::prepare()
{
	if (m_scene.changed())
	{
		// If render group already exist then ensure it doesn't contain anything
		// before begin re-created as it will otherwise destroy it's children.
		if (m_renderGroup)
		{
			m_renderGroup->removeAllEntities();
			m_renderGroup = 0;
		}

		// Create render entity group; contain scene root as well as dynamic entities.
		m_renderGroup = new world::GroupEntity();
		m_renderGroup->addEntity(m_scene->getRootEntity());
		m_renderGroup->addEntity(m_dynamicEntities);

		// Scene has been successfully validated; drop existing world renderer if we've been flushed.
		m_worldRenderer = 0;
		m_scene.consume();
	}

	// Re-create world renderer.
	if (!m_worldRenderer)
	{
		createWorldRenderer();
		if (!m_worldRenderer)
			return;
	}
}

void WorldLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_worldRenderer)
		return;

	// Update scene controller.
	m_scene->update(
		info.getSimulationTime(),
		info.getSimulationDeltaTime(),
		m_alternateTime,
		m_controllerEnable,
		false
	);

	// Update all entities; calling manually because we have exclusive control
	// of dynamic entities and an explicit render root group.
	world::Entity::UpdateParams up;
	up.totalTime = info.getSimulationTime();
	up.deltaTime = info.getSimulationDeltaTime();
	up.alternateTime = m_alternateTime;
	m_renderGroup->update(up);

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();
}

void WorldLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_worldRenderer)
		return;

	// Get camera entity and extract view transform.
	world::NullEntity* cameraEntity = m_scene->getEntitySchema()->getEntity< world::NullEntity >(L"Camera");
	if (cameraEntity)
	{
		Transform view = cameraEntity->getTransform(info.getInterval());
		m_worldRenderView.setView(view.inverse().toMatrix44());
	}

	// Build frame through world renderer.
	m_worldRenderView.setTimes(
		info.getStateTime(),
		info.getFrameDeltaTime(),
		info.getInterval()
	);
	m_worldRenderer->build(
		m_worldRenderView,
		m_renderGroup,
		frame
	);

	m_deltaTime = info.getFrameDeltaTime();
}

void WorldLayer::render(render::EyeType eye, uint32_t frame)
{
	if (!m_worldRenderer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	if (m_worldRenderer->begin(frame, eye, c_clearColor))
	{
		m_worldRenderer->render(
			world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap,
			frame,
			eye
		);
		m_worldRenderer->render(
			world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
			frame,
			eye
		);
		m_worldRenderer->end(frame, eye, m_deltaTime);
	}
}

void WorldLayer::leave()
{
	m_scene.clear();
	m_entities.clear();

	safeDestroy(m_renderGroup);
	safeDestroy(m_dynamicEntities);
	safeDestroy(m_worldRenderer);
}

void WorldLayer::reconfigured()
{
	createWorldRenderer();
}

Ref< world::EntityData > WorldLayer::getEntityData(const std::wstring& name) const
{
	std::map< std::wstring, resource::Proxy< world::EntityData > >::const_iterator i = m_entities.find(name);
	if (i != m_entities.end())
		return DeepClone(i->second.getResource()).create< world::EntityData >();
	else
		return 0;
}

world::Entity* WorldLayer::getEntity(const std::wstring& name) const
{
	return m_scene->getEntitySchema()->getEntity(name);
}

RefArray< world::Entity > WorldLayer::getEntities(const std::wstring& name) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(name, entities);
	return entities;
}

RefArray< world::Entity > WorldLayer::getEntitiesOf(const TypeInfo& entityType) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entityType, entities);
	return entities;
}

Ref< world::Entity > WorldLayer::createEntity(const std::wstring& name, world::IEntitySchema* entitySchema)
{
	std::map< std::wstring, resource::Proxy< world::EntityData > >::iterator i = m_entities.find(name);
	if (i == m_entities.end())
		return 0;

	world::IEntityBuilder* entityBuilder = m_environment->getWorld()->getEntityBuilder();
	T_ASSERT (entityBuilder);

	entityBuilder->begin(entitySchema);
	Ref< world::Entity > entity = entityBuilder->create(i->second);
	entityBuilder->end();

	return entity;
}

int32_t WorldLayer::getEntityIndex(const world::Entity* entity) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entities);

	RefArray< world::Entity >::const_iterator i = std::find(entities.begin(), entities.end(), entity);
	if (i == entities.end())
		return -1;

	return std::distance< RefArray< world::Entity >::const_iterator >(entities.begin(), i);
}

int32_t WorldLayer::getEntityIndexOf(const world::Entity* entity) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(type_of(entity), entities);

	RefArray< world::Entity >::const_iterator i = std::find(entities.begin(), entities.end(), entity);
	if (i == entities.end())
		return -1;

	return std::distance< RefArray< world::Entity >::const_iterator >(entities.begin(), i);
}

world::Entity* WorldLayer::getEntityByIndex(int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(index);
}

world::Entity* WorldLayer::getEntityOf(const TypeInfo& entityType, int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(entityType, index);
}

void WorldLayer::addEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(entity);
}

void WorldLayer::addTransientEntity(world::Entity* entity, float duration)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(new world::TransientEntity(m_dynamicEntities, entity, duration));
}

void WorldLayer::removeEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->removeEntity(entity);
}

world::IEntitySchema* WorldLayer::getEntitySchema() const
{
	return m_scene->getEntitySchema();
}

void WorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
}

world::PostProcess* WorldLayer::getPostProcess() const
{
	return m_worldRenderer->getVisualPostProcess();
}

bool WorldLayer::worldToView(const Vector4& worldPosition, Vector4& outViewPosition) const
{
	outViewPosition = m_worldRenderView.getView() * worldPosition.xyz1();
	return true;
}

bool WorldLayer::viewToWorld(const Vector4& viewPosition, Vector4& outWorldPosition) const
{
	outWorldPosition = m_worldRenderView.getView().inverse() * viewPosition.xyz1();
	return true;
}

bool WorldLayer::worldToScreen(const Vector4& worldPosition, Vector2& outScreenPosition) const
{
	Vector4 viewPosition = m_worldRenderView.getView() * worldPosition.xyz1();
	return viewToScreen(viewPosition, outScreenPosition);
}

bool WorldLayer::viewToScreen(const Vector4& viewPosition, Vector2& outScreenPosition) const
{
	Vector4 clipPosition = m_worldRenderView.getProjection() * viewPosition.xyz1();
	if (clipPosition.w() <= 0.0f)
		return false;
	clipPosition /= clipPosition.w();
	outScreenPosition = Vector2(clipPosition.x(), clipPosition.y());
	return true;
}

void WorldLayer::setFieldOfView(float fieldOfView)
{
	if (fieldOfView != m_fieldOfView)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		T_ASSERT (renderView);

		// Get render view dimensions.
		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		// Create world view.
		world::WorldViewPerspective worldViewPort;
		worldViewPort.width = width;
		worldViewPort.height = height;
		worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
		worldViewPort.fov = deg2rad(fieldOfView);
		m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);

		// Save field of view value as we must be able to re-create
		// world view if view port dimensions change.
		m_fieldOfView = fieldOfView;
	}
}

float WorldLayer::getFieldOfView() const
{
	return m_fieldOfView;
}

void WorldLayer::setAlternateTime(float alternateTime)
{
	m_alternateTime = alternateTime;
}

float WorldLayer::getAlternateTime() const
{
	return m_alternateTime;
}

void WorldLayer::createWorldRenderer()
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	// Destroy previous world renderer.
	safeDestroy(m_worldRenderer);

	// Get render view dimensions.
	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create world renderer.
	m_worldRenderer = m_environment->getWorld()->createWorldRenderer(
		m_scene->getWorldRenderSettings(),
		m_scene->getPostProcessSettings()
	);
	if (!m_worldRenderer)
		return;

	// Create world render view.
	world::WorldViewPerspective worldViewPort;
	worldViewPort.width = width;
	worldViewPort.height = height;
	worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
	worldViewPort.fov = deg2rad(m_fieldOfView);
	m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);
}

	}
}
