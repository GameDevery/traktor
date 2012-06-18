#ifndef traktor_animation_SkeletonFormatBvh_H
#define traktor_animation_SkeletonFormatBvh_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

class BvhDocument;

class SkeletonFormatBvh : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
	Ref< Skeleton > create(const BvhDocument* document, const Vector4& offset, float radius) const;

	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const;
};

	}
}

#endif	// traktor_animation_SkeletonFormatBvh_H
