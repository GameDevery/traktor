#pragma once

#include "Core/Math/Color4f.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace terrain
	{

class ColorBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	ColorBrush(drawing::Image* colorImage);

	virtual uint32_t begin(float x, float y, const State& state) override final;

	virtual void apply(float x, float y) override final;

	virtual void end(float x, float y) override final;

private:
	Ref< drawing::Image > m_colorImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	Color4f m_color;
};

	}
}

