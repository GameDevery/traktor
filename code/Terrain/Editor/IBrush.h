#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4f.h"

namespace traktor
{
	namespace terrain
	{

class IFallOff;

class IBrush : public Object
{
	T_RTTI_CLASS;

public:
	enum Mode
	{
		MdSplat = 1,
		MdColor = 2,
		MdHeight = 4,
		MdCut = 8,
		MdMaterial = 16
	};

	struct State
	{
		int32_t radius;
		const IFallOff* falloff;
		float strength;
		Color4f color;
		int32_t material;
		int32_t attribute;

		State()
		:	radius(0)
		,	falloff(nullptr)
		,	strength(0.0f)
		,	material(0)
		,	attribute(0)
		{
		}
	};

	virtual uint32_t begin(float x, float y, const State& state) = 0;

	virtual void apply(float x, float y) = 0;

	virtual void end(float x, float y) = 0;
};

	}
}

