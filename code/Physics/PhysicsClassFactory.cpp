/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Physics/BallJoint.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/ConeTwistJoint.h"
#include "Physics/Hinge2Joint.h"
#include "Physics/HingeJoint.h"
#include "Physics/PhysicsClassFactory.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Vehicle/VehicleComponent.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

class CollisionContactWrapper : public Object
{
	T_RTTI_CLASS;

public:
	CollisionContactWrapper(const AlignedVector< CollisionContact >& contacts)
	:	m_contacts(contacts)
	{
	}

	int32_t length() const
	{
		return int32_t(m_contacts.size());
	}

	const Vector4& position(int32_t index) const
	{
		return m_contacts[index].position;
	}

	const Vector4& normal(int32_t index) const
	{
		return m_contacts[index].normal;
	}

	float depth(int32_t index) const
	{
		return m_contacts[index].depth;
	}

private:
	const AlignedVector< CollisionContact >& m_contacts;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CollisionContact", CollisionContactWrapper, Object)

class CollisionListenerWrapper : public CollisionListener
{
	T_RTTI_CLASS;

public:
	CollisionListenerWrapper(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(const CollisionInfo& collisionInfo) override final
	{
		Any argv[] =
		{
			Any::fromObject(collisionInfo.body1),
			Any::fromObject(collisionInfo.body2),
			Any::fromObject(new CollisionContactWrapper(collisionInfo.contacts))
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CollisionListener", CollisionListenerWrapper, CollisionListener)

class QueryFilterWrapper : public Object
{
	T_RTTI_CLASS;

public:
	QueryFilterWrapper(uint32_t includeGroup)
	:	m_queryFilter(includeGroup)
	{
	}

	QueryFilterWrapper(uint32_t includeGroup, uint32_t ignoreGroup)
	:	m_queryFilter(includeGroup, ignoreGroup)
	{
	}

	QueryFilterWrapper(uint32_t includeGroup, uint32_t ignoreGroup, uint32_t ignoreClusterId)
	:	m_queryFilter(includeGroup, ignoreGroup, ignoreClusterId)
	{
	}

	operator const QueryFilter& () const
	{
		return m_queryFilter;
	}

private:
	QueryFilter m_queryFilter;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.QueryFilter", QueryFilterWrapper, Object)

class QueryResultWrapper : public Object
{
	T_RTTI_CLASS;

public:
	QueryResultWrapper(const QueryResult& result)
	:	m_result(result)
	{
	}

	Body* body() const
	{
		return m_result.body;
	}

	const Vector4& position() const
	{
		return m_result.position;
	}

	const Vector4& normal() const
	{
		return m_result.normal;
	}

	float distance() const
	{
		return m_result.distance;
	}

	float fraction() const
	{
		return m_result.fraction;
	}

	int32_t material() const
	{
		return m_result.material;
	}

private:
	QueryResult m_result;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.QueryResult", QueryResultWrapper, Object)

class BodyStateWrapper : public Object
{
	T_RTTI_CLASS;

public:
	BodyStateWrapper()
	{
	}

	BodyStateWrapper(const BodyState& state)
	:	m_state(state)
	{
	}

	operator const BodyState& () const
	{
		return m_state;
	}

private:
	BodyState m_state;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyState", BodyStateWrapper, Object)

Ref< QueryResultWrapper > PhysicsManager_queryPoint(PhysicsManager* this_, const Vector4& at, float margin)
{
	QueryResult result;
	if (this_->queryPoint(at, margin, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

Ref< QueryResultWrapper > PhysicsManager_queryRay(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter,
	bool ignoreBackFace
)
{
	QueryResult result;
	if (this_->queryRay(at, direction, maxLength, *queryFilter, ignoreBackFace, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

bool PhysicsManager_queryShadowRay(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter,
	uint32_t queryTypes
)
{
	return this_->queryShadowRay(at, direction, maxLength, *queryFilter, queryTypes);
}

RefArray< Body > PhysicsManager_querySphere(
	PhysicsManager* this_,
	const Vector4& at,
	float radius,
	const QueryFilterWrapper* queryFilter,
	uint32_t queryTypes
)
{
	RefArray< Body > bodies;
	this_->querySphere(at, radius, *queryFilter, queryTypes, bodies);
	return bodies;
}

Ref< QueryResultWrapper > PhysicsManager_querySweep_1(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilterWrapper* queryFilter
)
{
	QueryResult result;
	if (this_->querySweep(at, direction, maxLength, radius, *queryFilter, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

Ref< QueryResultWrapper > PhysicsManager_querySweep_2(
	PhysicsManager* this_,
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter
)
{
	QueryResult result;
	if (this_->querySweep(body, orientation, at, direction, maxLength, *queryFilter, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

bool Body_setState(Body* this_, const BodyStateWrapper* state)
{
	if (state)
		return this_->setState(*state);
	else
		return false;
}

Ref< BodyStateWrapper > Body_getState(Body* this_)
{
	return new BodyStateWrapper(this_->getState());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsClassFactory", 0, PhysicsClassFactory, IRuntimeClassFactory)

void PhysicsClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< QueryFilterWrapper > > classQueryFilter = new AutoRuntimeClass< QueryFilterWrapper >();
	classQueryFilter->addConstructor< uint32_t >();
	classQueryFilter->addConstructor< uint32_t, uint32_t >();
	classQueryFilter->addConstructor< uint32_t, uint32_t, uint32_t >();
	registrar->registerClass(classQueryFilter);

	Ref< AutoRuntimeClass< QueryResultWrapper > > classQueryResult = new AutoRuntimeClass< QueryResultWrapper >();
	classQueryResult->addProperty("body", &QueryResultWrapper::body);
	classQueryResult->addProperty("position", &QueryResultWrapper::position);
	classQueryResult->addProperty("normal", &QueryResultWrapper::normal);
	classQueryResult->addProperty("distance", &QueryResultWrapper::distance);
	classQueryResult->addProperty("fraction", &QueryResultWrapper::fraction);
	classQueryResult->addProperty("material", &QueryResultWrapper::material);
	registrar->registerClass(classQueryResult);

	Ref< AutoRuntimeClass< BodyStateWrapper > > classBodyState = new AutoRuntimeClass< BodyStateWrapper >();
	registrar->registerClass(classBodyState);

	Ref< AutoRuntimeClass< PhysicsManager > > classPhysicsManager = new AutoRuntimeClass< PhysicsManager >();
	classPhysicsManager->addProperty("gravity", &PhysicsManager::setGravity, &PhysicsManager::getGravity);
	classPhysicsManager->addProperty("bodies", &PhysicsManager::getBodies);
	classPhysicsManager->addMethod("addCollisionListener", &PhysicsManager::addCollisionListener);
	classPhysicsManager->addMethod("removeCollisionListener", &PhysicsManager::removeCollisionListener);
	classPhysicsManager->addMethod("update", &PhysicsManager::update);
	classPhysicsManager->addMethod("queryPoint", &PhysicsManager_queryPoint);
	classPhysicsManager->addMethod("queryRay", &PhysicsManager_queryRay);
	classPhysicsManager->addMethod("queryShadowRay", &PhysicsManager_queryShadowRay);
	classPhysicsManager->addMethod("querySphere", &PhysicsManager_querySphere);
	classPhysicsManager->addMethod("querySweep", &PhysicsManager_querySweep_1);
	classPhysicsManager->addMethod("querySweep", &PhysicsManager_querySweep_2);
	registrar->registerClass(classPhysicsManager);

	Ref< AutoRuntimeClass< Body > > classBody = new AutoRuntimeClass< Body >();
	classBody->addProperty("transform", &Body::setTransform, &Body::getTransform);
	classBody->addProperty("centerTransform", &Body::getCenterTransform);
	classBody->addProperty("static", &Body::isStatic);
	classBody->addProperty("kinematic", &Body::isKinematic);
	classBody->addProperty("active", &Body::setActive, &Body::isActive);
	classBody->addProperty("enable", &Body::setEnable, &Body::isEnable);
	classBody->addProperty("linearVelocity", &Body::setLinearVelocity, &Body::getLinearVelocity);
	classBody->addProperty("angularVelocity", &Body::setAngularVelocity, &Body::getAngularVelocity);
	classBody->addProperty("userObject", &Body::setUserObject, &Body::getUserObject);
	classBody->addMethod("reset", &Body::reset);
	classBody->addMethod("setMass", &Body::setMass);
	classBody->addMethod("getInverseMass", &Body::getInverseMass);
	classBody->addMethod("addForceAt", &Body::addForceAt);
	classBody->addMethod("addTorque", &Body::addTorque);
	classBody->addMethod("addLinearImpulse", &Body::addLinearImpulse);
	classBody->addMethod("addAngularImpulse", &Body::addAngularImpulse);
	classBody->addMethod("addImpulse", &Body::addImpulse);
	classBody->addMethod("getVelocityAt", &Body::getVelocityAt);
	classBody->addMethod("setState", &Body_setState);
	classBody->addMethod("getState", &Body_getState);
	classBody->addMethod("addCollisionListener", &Body::addCollisionListener);
	classBody->addMethod("removeCollisionListener", &Body::removeCollisionListener);
	classBody->addMethod("removeAllCollisionListeners", &Body::removeAllCollisionListeners);
	registrar->registerClass(classBody);

	Ref< AutoRuntimeClass< Joint > > classJoint = new AutoRuntimeClass< Joint >();
	classJoint->addProperty("body1", &Joint::getBody1);
	classJoint->addProperty("body2", &Joint::getBody2);
	registrar->registerClass(classJoint);

	Ref< AutoRuntimeClass< BallJoint > > classBallJoint = new AutoRuntimeClass< BallJoint >();
	classBallJoint->addProperty("anchor", &BallJoint::setAnchor, &BallJoint::getAnchor);
	registrar->registerClass(classBallJoint);

	Ref< AutoRuntimeClass< ConeTwistJoint > > classConeTwistJoint = new AutoRuntimeClass< ConeTwistJoint >();
	registrar->registerClass(classConeTwistJoint);

	Ref< AutoRuntimeClass< Hinge2Joint > > classHinge2Joint = new AutoRuntimeClass< Hinge2Joint >();
	classHinge2Joint->addMethod("addTorques", &Hinge2Joint::addTorques);
	classHinge2Joint->addMethod("getAngleAxis1", &Hinge2Joint::getAngleAxis1);
	classHinge2Joint->addMethod("setVelocityAxis1", &Hinge2Joint::setVelocityAxis1);
	classHinge2Joint->addMethod("setVelocityAxis2", &Hinge2Joint::setVelocityAxis2);
	registrar->registerClass(classHinge2Joint);

	Ref< AutoRuntimeClass< HingeJoint > > classHingeJoint = new AutoRuntimeClass< HingeJoint >();
	classHingeJoint->addProperty("anchor", &HingeJoint::getAnchor);
	classHingeJoint->addProperty("axis", &HingeJoint::getAxis);
	classHingeJoint->addProperty("angle", &HingeJoint::getAngle);
	classHingeJoint->addProperty("angleVelocity", &HingeJoint::getAngleVelocity);
	registrar->registerClass(classHingeJoint);

	Ref< AutoRuntimeClass< ArticulatedEntity > > classArticulatedEntity = new AutoRuntimeClass< ArticulatedEntity >();
	classArticulatedEntity->addProperty("entities", &ArticulatedEntity::getEntities);
	classArticulatedEntity->addProperty("joints", &ArticulatedEntity::getJoints);
	registrar->registerClass(classArticulatedEntity);

	Ref< AutoRuntimeClass< RigidEntity > > classRigidEntity = new AutoRuntimeClass< RigidEntity >();
	classRigidEntity->addProperty("body", &RigidEntity::getBody);
	classRigidEntity->addProperty("entity", &RigidEntity::getEntity);
	registrar->registerClass(classRigidEntity);

	Ref< AutoRuntimeClass< CharacterComponent > > classCharacterComponent = new AutoRuntimeClass< CharacterComponent >();
	classCharacterComponent->addProperty("velocity", &CharacterComponent::setVelocity, &CharacterComponent::getVelocity);
	classCharacterComponent->addProperty("headAngle", &CharacterComponent::setHeadAngle, &CharacterComponent::getHeadAngle);
	classCharacterComponent->addProperty("grounded", &CharacterComponent::isGrounded);
	registrar->registerClass(classCharacterComponent);

	Ref< AutoRuntimeClass< RigidBodyComponent > > classRigidBodyComponent = new AutoRuntimeClass< RigidBodyComponent >();
	classRigidBodyComponent->addProperty("body", &RigidBodyComponent::getBody);
	registrar->registerClass(classRigidBodyComponent);

	Ref< AutoRuntimeClass< VehicleComponent > > classVehicleComponent = new AutoRuntimeClass< VehicleComponent >();
	classVehicleComponent->addProperty("steerAngle", &VehicleComponent::setSteerAngle, &VehicleComponent::getSteerAngle);
	classVehicleComponent->addProperty("engineThrottle", &VehicleComponent::setEngineThrottle, &VehicleComponent::getEngineThrottle);
	registrar->registerClass(classVehicleComponent);

	Ref< AutoRuntimeClass< CollisionContactWrapper > > classCollisionContact = new AutoRuntimeClass< CollisionContactWrapper >();
	classCollisionContact->addProperty("length", &CollisionContactWrapper::length);
	classCollisionContact->addMethod("position", &CollisionContactWrapper::position);
	classCollisionContact->addMethod("normal", &CollisionContactWrapper::normal);
	classCollisionContact->addMethod("depth", &CollisionContactWrapper::depth);
	registrar->registerClass(classCollisionContact);

	Ref< AutoRuntimeClass< CollisionListenerWrapper > > classCollisionListener = new AutoRuntimeClass< CollisionListenerWrapper >();
	registrar->registerClass(classCollisionListener);

	Ref< AutoRuntimeClass< CollisionListenerWrapper > > classCollisionListenerDelegate = new AutoRuntimeClass< CollisionListenerWrapper >();
	classCollisionListenerDelegate->addConstructor< IRuntimeDelegate* >();
	registrar->registerClass(classCollisionListenerDelegate);
}

	}
}
