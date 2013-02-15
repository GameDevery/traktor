#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Parade/WorldLayer.h"
#include "Parade/WorldLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.WorldLayerData", 0, WorldLayerData, LayerData)

Ref< Layer > WorldLayerData::createInstance(Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< scene::Scene > scene;
	std::map< std::wstring, resource::Proxy< world::EntityData > > entities;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_scene, scene))
		return 0;

	for (std::map< std::wstring, resource::Id< world::EntityData > >::const_iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (!resourceManager->bind(i->second, entities[i->first]))
			return 0;
	}

	// Create layer instance.
	return new WorldLayer(
		stage,
		m_name,
		environment,
		scene,
		entities
	);
}

bool WorldLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< scene::Scene >(L"scene", m_scene);
	s >> MemberStlMap<
		std::wstring,
		resource::Id< world::EntityData >,
		MemberStlPair<
			std::wstring,
			resource::Id< world::EntityData >,
			Member< std::wstring >,
			resource::Member< world::EntityData >
		>
	>(L"entities", m_entities);

	return true;
}

	}
}
