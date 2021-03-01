#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Joint;
class JointDesc;
class PhysicsManager;

/*! Joint component.
 * \ingroup Physics
 */
class T_DLLCLASS JointComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	JointComponent(PhysicsManager* physicsManager, const JointDesc* jointDesc);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	Ref< PhysicsManager > m_physicsManager;
	Ref< const JointDesc > m_jointDesc;
	world::Entity* m_owner;
	Ref< Joint > m_joint;
};

	}
}

