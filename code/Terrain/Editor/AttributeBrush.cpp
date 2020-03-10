#include "Core/Math/Const.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/AttributeBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.AttributeBrush", AttributeBrush, IBrush)

AttributeBrush::AttributeBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_attribute(0)
{
}

uint32_t AttributeBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t attribute)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = powf(abs(strength), 2.0f);
	m_attribute = attribute;
	return MdMaterial;
}

void AttributeBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			m_heightfield->setGridMaterial(x + ix, y + iy, m_attribute);
		}
	}
}

void AttributeBrush::end(int32_t x, int32_t y)
{
}

	}
}
