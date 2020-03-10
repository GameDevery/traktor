#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ElevateBrush", ElevateBrush, IBrush)

ElevateBrush::ElevateBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage)
:	m_heightfield(heightfield)
,	m_splatImage(splatImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_attribute(-1)
{
}

uint32_t ElevateBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t attribute)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = strength * 0.256f / m_heightfield->getWorldExtent().y();
	m_attribute = attribute;
	return MdHeight;
}

void ElevateBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			int32_t gx = x + ix;
			int32_t gy = y + iy;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;

			// Check material mask.
			if (m_attribute >= 0)
			{
				Color4f targetColor;
				m_splatImage->getPixel(gx, gy, targetColor);

				float T_MATH_ALIGN16 weights[4];
				targetColor.storeAligned(weights);

				a *= 1.0f - weights[m_attribute];
			}

			if (abs(a) <= FUZZY_EPSILON)
				continue;

			float h = m_heightfield->getGridHeightNearest(gx, gy);
			m_heightfield->setGridHeight(gx, gy, h + a);
		}
	}
}

void ElevateBrush::end(int32_t x, int32_t y)
{
}

	}
}
