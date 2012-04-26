#ifndef traktor_animation_ISkeletonFormat_H
#define traktor_animation_ISkeletonFormat_H

#include "Core/Object.h"

namespace traktor
{

class IStream;
class Vector4;

	namespace animation
	{

class Skeleton;

class ISkeletonFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float boneRadius, bool invertX, bool invertZ) const = 0;
};

	}
}

#endif	// traktor_animation_ISkeletonFormat_H
