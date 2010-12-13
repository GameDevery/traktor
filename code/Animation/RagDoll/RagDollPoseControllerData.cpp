#include "Animation/SkeletonUtils.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollPoseControllerData", 4, RagDollPoseControllerData, IPoseControllerData)

RagDollPoseControllerData::RagDollPoseControllerData()
:	m_collisionGroup(~0UL)
,	m_autoDeactivate(true)
,	m_enabled(true)
,	m_fixateBones(false)
,	m_limbMass(1.0f)
,	m_linearDamping(0.1f)
,	m_angularDamping(0.1f)
,	m_linearThreshold(0.8f)
,	m_angularThreshold(1.0f)
,	m_trackLinearTension(0.0f)
,	m_trackAngularTension(0.0f)
,	m_trackDuration(0.0f)
{
}

Ref< IPoseController > RagDollPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
)
{
	Ref< IPoseController > trackPoseController;

	AlignedVector< Transform > boneTransforms;
	calculateBoneTransforms(skeleton, boneTransforms);

	AlignedVector< IPoseController::Velocity > velocities;
	velocities.reserve(boneTransforms.size());

	if (m_trackPoseController)
	{
		trackPoseController = m_trackPoseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			worldTransform
		);
		if (!trackPoseController)
			return 0;
			
		trackPoseController->estimateVelocities(skeleton, velocities);
	}

	for (uint32_t i = velocities.size(); i < boneTransforms.size(); ++i)
	{
		IPoseController::Velocity velocity;
		velocity.linear =
		velocity.angular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		velocities.push_back(velocity);
	}
	
	Ref< RagDollPoseController > poseController = new RagDollPoseController();
	if (!poseController->create(
		physicsManager,
		skeleton,
		worldTransform,
		boneTransforms,
		velocities,
		m_collisionGroup,
		m_autoDeactivate,
		m_enabled,
		m_fixateBones,
		m_limbMass,
		m_linearDamping,
		m_angularDamping,
		m_linearThreshold,
		m_angularThreshold,
		trackPoseController,
		m_trackLinearTension,
		m_trackAngularTension,
		m_trackDuration
	))
		return 0;

	return poseController;
}

bool RagDollPoseControllerData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
	{
		s >> Member< uint32_t >(L"collisionGroup", m_collisionGroup);
		s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
		s >> Member< bool >(L"enabled", m_enabled);
		s >> Member< bool >(L"fixateBones", m_fixateBones);
		s >> Member< float >(L"limbMass", m_limbMass);
		s >> Member< float >(L"linearDamping", m_linearDamping);
		s >> Member< float >(L"angularDamping", m_angularDamping);
	}
	if (s.getVersion() >= 4)
	{
		s >> Member< float >(L"linearThreshold", m_linearThreshold);
		s >> Member< float >(L"angularThreshold", m_angularThreshold);
	}
	if (s.getVersion() >= 2)
	{
		s >> MemberRef< IPoseControllerData >(L"trackPoseController", m_trackPoseController);
		s >> Member< float >(L"trackLinearTension", m_trackLinearTension);
		s >> Member< float >(L"trackAngularTension", m_trackAngularTension);
	}
	if (s.getVersion() >= 3)
	{
		s >> Member< float >(L"trackDuration", m_trackDuration);
	}
	return true;
}

	}
}
