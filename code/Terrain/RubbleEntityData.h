#ifndef traktor_terrain_RubbleEntityData_H
#define traktor_terrain_RubbleEntityData_H

#include "Resource/Id.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IEntityBuilder;
class Entity;

	}

	namespace terrain
	{

class Terrain;

class T_DLLCLASS RubbleEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	RubbleEntityData();

	Ref< world::Entity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(ISerializer& s);

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

	world::EntityData* getEntityData() const { return m_entityData; }

private:
	resource::Id< Terrain > m_terrain;
	Ref< world::EntityData > m_entityData;
	uint32_t m_count;
};

	}
}

#endif	// traktor_terrain_RubbleEntityData_H
