#pragma once

#include "Core/Thread/Semaphore.h"
#include "Physics/CollisionListener.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysX/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace physx
{

class PxPhysics;
class PxScene;

}

namespace traktor
{
	namespace physics
	{

class BodyPhysX;

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS PhysicsManagerPhysX
:	public PhysicsManager
,	public IWorldCallback
{
	T_RTTI_CLASS;

public:
	PhysicsManagerPhysX();

	virtual ~PhysicsManagerPhysX();

	virtual bool create(const PhysicsCreateDesc& desc);

	virtual void destroy();

	virtual void setGravity(const Vector4& gravity);

	virtual Vector4 getGravity() const;

	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag);

	virtual Ref< Joint > createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2);

	virtual void update(float simulationDeltaTime, bool issueCollisionEvents);

	virtual void solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints);

	virtual RefArray< Body > getBodies() const;

	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const;

	virtual bool queryPoint(
		const Vector4& at,
		float margin,
		QueryResult& outResult
	) const;

	virtual bool queryRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t ignoreClusterId,
		bool ignoreBackFace,
		QueryResult& outResult
	) const;

	virtual bool queryShadowRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t queryTypes,
		uint32_t ignoreClusterId
	) const;

	virtual uint32_t querySphere(
		const Vector4& at,
		float radius,
		uint32_t group,
		uint32_t queryTypes,
		RefArray< Body >& outBodies
	) const;

	virtual bool querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		uint32_t ignoreClusterId,
		QueryResult& outResult
	) const;

	virtual bool querySweep(
		const Body* body,
		const Quaternion& orientation,
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t ignoreClusterId,
		QueryResult& outResult
	) const;

	virtual void querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		uint32_t ignoreClusterId,
		AlignedVector< QueryResult >& outResult
	) const;

	virtual void queryOverlap(
		const Body* body,
		RefArray< Body >& outResult
	) const;

	virtual void queryTriangles(
		const Vector4& center,
		float radius,
		AlignedVector< TriangleResult >& outTriangles
	) const;

	virtual void getStatistics(PhysicsStatistics& outStatistics) const;

private:
	mutable Semaphore m_lock;
	float m_timeScale;
	physx::PxPhysics* m_sdk;
	physx::PxCooking* m_cooking;
	physx::PxScene* m_scene;
	RefArray< BodyPhysX > m_bodies;
	AlignedVector< CollisionInfo > m_collisionInfo;

	virtual void insertActor(physx::PxRigidActor* actor);

	virtual void removeActor(physx::PxRigidActor* actor);

	virtual void destroyBody(Body* owner, physx::PxRigidActor* actor);

	virtual void destroyJoint(Joint* owner, physx::PxJoint* joint);
};

	}
}

