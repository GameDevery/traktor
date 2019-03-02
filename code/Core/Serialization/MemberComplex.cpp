#include <string>
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

MemberComplex::MemberComplex(const wchar_t* const name, bool compound)
:	m_name(name)
,	m_compound(compound)
,	m_attributes(0)
{
}

MemberComplex::MemberComplex(const wchar_t* const name, bool compound, const Attribute& attributes)
:	m_name(name)
,	m_compound(compound)
,	m_attributes(&attributes)
{
}

MemberComplex::~MemberComplex()
{
}

}
