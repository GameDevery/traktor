#pragma once

namespace traktor
{
	namespace spray
	{

#pragma pack(1)
struct EmitterVertex
{
	float positionAndOrientation[4];
	float velocityAndRandom[4];
	float extentAlphaAndSize[4];
	float colorAndAge[4];
};
#pragma pack()

#pragma pack(1)
struct TrailVertex
{
	float position[4];
	float direction[4];
	float uv[4];
};
#pragma pack()

	}
}

