#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatBvh.h"
#include "Animation/Editor/BvhParser/BvhDocument.h"
#include "Animation/Editor/BvhParser/BvhJoint.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

void createJoints(
	Skeleton* skeleton,
	const BvhJoint* bvhJoint,
	int32_t parent,
	const Vector4& offset,
	float radius
)
{
	Ref< Joint > joint = new Joint();

	joint->setParent(parent);
	joint->setName(bvhJoint->getName());
	joint->setTransform(Transform(bvhJoint->getOffset() + offset));
	joint->setRadius(radius);
	joint->setEnableLimits(false);

	if (bvhJoint->getName().empty() && parent >= 0)
		joint->setName(skeleton->getJoint(parent)->getName() + L"_END");

	int32_t jointIndex = skeleton->addJoint(joint);

	const RefArray< BvhJoint >& children = bvhJoint->getChildren();
	for (RefArray< BvhJoint >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		const BvhJoint* childBvhJoint = *i;
		T_ASSERT (childBvhJoint);

		createJoints(skeleton, childBvhJoint, jointIndex, Vector4::zero(), radius);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatBvh", SkeletonFormatBvh, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatBvh::create(const BvhDocument* document, const Vector4& offset, float radius) const
{
	Ref< Skeleton > skeleton = new Skeleton();
	createJoints(
		skeleton,
		document->getRootJoint(),
		-1,
		offset,
		radius
	);
	return skeleton;
}

Ref< Skeleton > SkeletonFormatBvh::import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const
{
	Vector4 jointModifier(
		invertX ? -1.0f : 1.0f, 
		1.0f, 
		invertZ ? -1.0f : 1.0f, 
		1.0f
	);

	Ref< BvhDocument > document = BvhDocument::parse(stream, jointModifier);
	if (!document)
		return 0;

	Ref< Skeleton > skeleton = create(document, offset, radius);

	if (skeleton)
		log::info << L"Created " << skeleton->getJointCount() << L" joints(s) in skeleton" << Endl;

	return skeleton;
}

	}
}
