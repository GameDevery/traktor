#pragma once

#include "Physics/FixedJoint.h"
#include "Physics/PhysX/JointPhysX.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS FixedJointPhysX : public JointPhysX< FixedJoint >
{
	T_RTTI_CLASS;

public:
	FixedJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2);
};

	}
}

