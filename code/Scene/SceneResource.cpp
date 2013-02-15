#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/WorldRenderSettings.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneResource", 0, SceneResource, ISerializable)

SceneResource::SceneResource()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

Ref< Scene > SceneResource::createScene(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder,
	world::IEntitySchema* entitySchema
) const
{
	resource::Proxy< world::PostProcessSettings > postProcessSettings;

	if (!m_postProcessSettings.isNull())
	{
		if (!resourceManager->bind(m_postProcessSettings, postProcessSettings))
			log::error << L"Unable to bind post processing settings" << Endl;
	}

	entityBuilder->begin(entitySchema);

	Ref< world::Entity > rootEntity = entityBuilder->create(m_entityData);

	Ref< ISceneController > controller;
	if (m_controllerData)
	{
		controller = m_controllerData->createController(entityBuilder, entitySchema);
		if (!controller)
		{
			log::error << L"Failed to create scene; unable to instantiate scene controller" << Endl;
			return 0;
		}
	}

	entityBuilder->end();

	return new Scene(
		controller,
		entitySchema,
		rootEntity,
		m_worldRenderSettings,
		postProcessSettings
	);
}

void SceneResource::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

Ref< world::WorldRenderSettings > SceneResource::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneResource::setPostProcessSettings(const resource::Id< world::PostProcessSettings >& postProcessSettings)
{
	m_postProcessSettings = postProcessSettings;
}

const resource::Id< world::PostProcessSettings >& SceneResource::getPostProcessSettings() const
{
	return m_postProcessSettings;
}

void SceneResource::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > SceneResource::getEntityData() const
{
	return m_entityData;
}

void SceneResource::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneResource::getControllerData() const
{
	return m_controllerData;
}

bool SceneResource::serialize(ISerializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> resource::Member< world::PostProcessSettings >(L"postProcessSettings", m_postProcessSettings);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
	return true;
}

	}
}
