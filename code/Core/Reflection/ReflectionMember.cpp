#include "Core/Reflection/ReflectionMember.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ReflectionMember", ReflectionMember, Object)

const wchar_t* ReflectionMember::getName() const
{
	return m_name;
}

ReflectionMember::ReflectionMember(const wchar_t* name)
:	m_name(name)
{
}

}
