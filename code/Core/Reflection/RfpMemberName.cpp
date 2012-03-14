#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberName.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpMemberName", RfpMemberName, ReflectionMemberPredicate)

RfpMemberName::RfpMemberName(const std::wstring& memberName)
:	m_memberName(memberName)
{
}

bool RfpMemberName::operator () (const ReflectionMember* member) const
{
	T_ASSERT (member);
	return member->getName() == m_memberName;
}

}
