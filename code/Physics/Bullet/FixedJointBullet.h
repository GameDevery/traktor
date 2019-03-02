#pragma once

#include "Physics/FixedJoint.h"
#include "Physics/Bullet/JointBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS FixedJointBullet : public JointBullet< FixedJoint, btGeneric6DofConstraint >
{
	T_RTTI_CLASS;

public:
	FixedJointBullet(IWorldCallback* callback, btGeneric6DofConstraint* constraint, BodyBullet* body1, BodyBullet* body2);
};

	}
}

