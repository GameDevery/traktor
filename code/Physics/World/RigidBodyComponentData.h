#pragma once

#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
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

class Entity;
class IEntityBuilder;
class IEntityEventData;
class EntityEventManager;

	}

	namespace physics
	{

class BodyDesc;
class PhysicsManager;
class RigidBodyComponent;

/*!
 * \ingroup Physics
 */
class T_DLLCLASS RigidBodyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	RigidBodyComponentData();

	explicit RigidBodyComponentData(BodyDesc* bodyDesc);

	explicit RigidBodyComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide);

	Ref< RigidBodyComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		world::EntityEventManager* eventManager,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	const world::IEntityEventData* getEventCollide() const { return m_eventCollide; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::IEntityEventData > m_eventCollide;
};

	}
}

