#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class Color4f;

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

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) = 0;

	virtual void apply(int32_t x, int32_t y) = 0;

	virtual void end(int32_t x, int32_t y) = 0;
};

	}
}

