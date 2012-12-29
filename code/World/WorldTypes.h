#ifndef traktor_world_WorldTypes_H
#define traktor_world_WorldTypes_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace world
	{

enum
{
	MaxSliceCount = 4,
	MaxLightCount = 16
};

enum Quality
{
	QuDisabled = 0,
	QuLow = 1,
	QuMedium = 2,
	QuHigh = 3,
	QuUltra = 4,
	QuLast = 5
};

enum LightType
{
	LtDisabled = 0,
	LtDirectional = 1,
	LtPoint = 2,
	LtSpot = 3
};

struct Light
{
	LightType type;
	Vector4 position;
	Vector4 direction;
	Vector4 sunColor;
	Vector4 baseColor;
	Vector4 shadowColor;
	Scalar range;
	Scalar radius;
	bool castShadow;
};

	}
}

#endif	// traktor_world_WorldTypes_H
