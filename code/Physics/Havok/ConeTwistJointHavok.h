#pragma once

#include "Physics/ConeTwistJoint.h"
#include "Physics/Havok/JointHavok.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_HAVOK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Havok
 */
class T_DLLCLASS ConeTwistJointHavok : public JointHavok< ConeTwistJoint >
{
	T_RTTI_CLASS;

public:
	ConeTwistJointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2);
};

	}
}

