#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Flash/CharacterInstance.h"
#include "Flash/SwfMembers.h"
#include "Flash/Debug/InstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.InstanceDebugInfo", InstanceDebugInfo, ISerializable)

InstanceDebugInfo::InstanceDebugInfo()
:	m_visible(false)
{
}

void InstanceDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"name", m_name);
	s >> MemberAabb2(L"bounds", m_bounds);
	s >> Member< Matrix33 >(L"localTransform", m_localTransform);
	s >> Member< Matrix33 >(L"globalTransform", m_globalTransform);
	s >> MemberColorTransform(L"cxform", m_cxform);
	s >> Member< bool >(L"visible", m_visible);
}

	}
}
