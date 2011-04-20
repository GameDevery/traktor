#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/PointLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.PointLightEntityData", 2, PointLightEntityData, SpatialEntityData)

PointLightEntityData::PointLightEntityData()
:	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_range(0.0f)
,	m_randomFlickerAmount(0.0f)
,	m_randomFlickerFilter(0.5f)
{
}

bool PointLightEntityData::serialize(ISerializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);
	s >> Member< float >(L"range", m_range);

	if (s.getVersion() == 1)
		s >> Member< float >(L"randomFlicker", m_randomFlickerAmount, AttributeRange(0.0f, 1.0f));
	else if (s.getVersion() >= 2)
	{
		s >> Member< float >(L"randomFlickerAmount", m_randomFlickerAmount, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"randomFlickerFilter", m_randomFlickerFilter, AttributeRange(0.0f, 1.0f));
	}

	return true;
}

	}
}
