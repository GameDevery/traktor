#include "Core/Serialization/AttributeRange.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeRange", AttributeRange, Attribute)

AttributeRange::AttributeRange(float min, float max)
:	m_min(min)
,	m_max(max)
{
}

float AttributeRange::getMin() const
{
	return m_min;
}

float AttributeRange::getMax() const
{
	return m_max;
}

}
