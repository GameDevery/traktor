#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{

MemberType::MemberType(const wchar_t* const name, const TypeInfo*& type)
:	MemberComplex(name, false)
,	m_type(type)
{
}

bool MemberType::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::SdRead)
	{
		std::wstring name;
		s >> Member< std::wstring >(getName(), name);
		
		m_type = TypeInfo::find(name);
		return bool(m_type != 0);
	}
	else	// SdWrite
	{
		std::wstring name = m_type->getName();
		s >> Member< std::wstring >(getName(), name);
	}
	return true;
}

}
