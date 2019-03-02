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

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) override final;

	virtual void apply(int32_t x, int32_t y) override final;

	virtual void end(int32_t x, int32_t y) override final;

	virtual Ref< IBrush > clone() const override final;

	virtual bool contained() const  override final { return true; }

private:
	Ref< drawing::Image > m_colorImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	Color4f m_color;
};

	}
}

