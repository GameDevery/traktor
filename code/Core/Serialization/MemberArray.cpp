#include "Core/Serialization/MemberArray.h"

namespace traktor
{

MemberArray::MemberArray(const wchar_t* const name, const Attribute* attributes)
:	m_name(name)
,	m_attributes(attributes)
{
}

void MemberArray::setAttributes(const Attribute* attributes)
{
    m_attributes = attributes;
}

}
