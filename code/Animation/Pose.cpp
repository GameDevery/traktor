#include "Animation/Pose.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Pose", 0, Pose, ISerializable)

void Pose::setJointTransform(uint32_t jointIndex, const Transform& jointTransform)
{
	Joint& joint = getEditJoint(jointIndex);
	joint.transform = jointTransform;
}

Transform Pose::getJointTransform(uint32_t jointIndex) const
{
	const Joint* joint = getJoint(jointIndex);
	return joint ? joint->transform : Transform::identity();
}

uint32_t Pose::getMaxIndex() const
{
	uint32_t maxIndex =  0;
	for (AlignedVector< Joint >::const_iterator i = m_joints.begin(); i != m_joints.end(); ++i)
		maxIndex = max(maxIndex, i->index);
	return maxIndex;
}

void Pose::getIndexMask(BitSet& outIndices) const
{
	for (AlignedVector< Joint >::const_iterator i = m_joints.begin(); i != m_joints.end(); ++i)
		outIndices.set(i->index);
}

const Pose::Joint* Pose::getJoint(uint32_t jointIndex) const
{
	uint32_t s = 0;
	uint32_t e = uint32_t(m_joints.size());

	while (s < e)
	{
		uint32_t m = s + (e - s) / 2;
		if (jointIndex == m_joints[m].index)
			return &m_joints[m];
		else if (jointIndex < m_joints[m].index)
			e = m;
		else if (jointIndex > m_joints[m].index)
			s = m + 1;
	}

	return 0;
}

Pose::Joint& Pose::getEditJoint(uint32_t jointIndex)
{
	size_t s = 0;
	size_t e = m_joints.size();

	while (s < e)
	{
		size_t m = s + (e - s) / 2;
		if (jointIndex == m_joints[m].index)
			return m_joints[m];
		else if (jointIndex < m_joints[m].index)
			e = m;
		else if (jointIndex > m_joints[m].index)
			s = m + 1;
	}

	return *m_joints.insert(m_joints.begin() + s, Joint(jointIndex));
}

void Pose::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Joint, MemberComposite< Joint > >(L"joints", m_joints);
}

void Pose::Joint::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"index", index);
	s >> MemberComposite< Transform  >(L"transform", transform);
}

	}
}
