#ifndef traktor_animation_SkeletonFormatLws_H
#define traktor_animation_SkeletonFormatLws_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

class SkeletonFormatLws : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const;
};

	}
}

#endif	// traktor_animation_SkeletonFormatLws_H
