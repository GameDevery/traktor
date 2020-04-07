#pragma once

#include <set>
#include "Resource/Id.h"
#include "World/Entity/GroupEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace physics
    {

class CollisionSpecification;

    }

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS SplineEntityData : public world::GroupEntityData
{
	T_RTTI_CLASS;

public:
	SplineEntityData();

	void setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const;

    virtual void serialize(ISerializer& s) override final;

private:
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

	}
}
