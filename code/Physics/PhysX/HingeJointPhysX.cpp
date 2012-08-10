#include "Physics/PhysX/HingeJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HingeJointPhysX", HingeJointPhysX, HingeJoint)

HingeJointPhysX::HingeJointPhysX(DestroyCallbackPhysX* callback, NxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< HingeJoint >(callback, joint, body1, body2)
{
}

Vector4 HingeJointPhysX::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

Vector4 HingeJointPhysX::getAxis() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

float HingeJointPhysX::getAngle() const
{
	return 0.0f;
}

float HingeJointPhysX::getAngleVelocity() const
{
	return 0.0f;
}

	}
}
