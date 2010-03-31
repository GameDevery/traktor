#ifndef traktor_physics_ArticulatedEntityData_H
#define traktor_physics_ArticulatedEntityData_H

#include "Core/RefArray.h"
#include "World/Entity/SpatialEntityData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;

	}

	namespace physics
	{

class ArticulatedEntity;
class JointDesc;
class PhysicsManager;
class RigidEntityData;

/*! \brief Articulated entity data.
 * \ingroup Physics
 */
class T_DLLCLASS ArticulatedEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS;

public:
	struct Constraint
	{
		Ref< JointDesc > jointDesc;
		int32_t entityIndex1;
		int32_t entityIndex2;

		Constraint();

		bool serialize(ISerializer& s);
	};

	Ref< ArticulatedEntity > createEntity(
		world::IEntityBuilder* builder,
		PhysicsManager* physicsManager
	) const;

	virtual void setTransform(const Transform& transform);
	
	virtual bool serialize(ISerializer& s);

	const RefArray< RigidEntityData >& getEntityData() const { return m_entityData; }

	const std::vector< Constraint >& getConstraints() const { return m_constraints; }

private:
	RefArray< RigidEntityData > m_entityData;
	std::vector< Constraint > m_constraints;
};

	}
}

#endif	// traktor_physics_ArticulatedEntityData_H
