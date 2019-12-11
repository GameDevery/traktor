#pragma once

#include <set>
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
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

class CollisionSpecification;

/*! Collision shape description.
 * \ingroup Physics
 */
class T_DLLCLASS ShapeDesc : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum { Version = 6 };

	ShapeDesc();

	void setLocalTransform(const Transform& localTransform);

	const Transform& getLocalTransform() const;

	void setCollisionGroup(const std::set< resource::Id< CollisionSpecification > >& collisionGroup);

	const std::set< resource::Id< CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const std::set< resource::Id< CollisionSpecification > >& collisionMask);

	const std::set< resource::Id< CollisionSpecification > >& getCollisionMask() const;

	void setMaterial(int32_t material);

	int32_t getMaterial() const;

	virtual void serialize(ISerializer& s) override;

private:
	Transform m_localTransform;
	std::set< resource::Id< CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< CollisionSpecification > > m_collisionMask;
	int32_t m_material;
};

	}
}

