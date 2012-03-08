#include "Amalgam/Editor/Platform.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Platform", 0, Platform, ISerializable)

Platform::Platform()
:	m_iconIndex(0)
{
}

int32_t Platform::getIconIndex() const
{
	return m_iconIndex;
}

const DeployTool& Platform::getDeployTool() const
{
#if TARGET_OS_MAC
	return m_deployToolOsX;
#else
	return m_deployToolWin32;
#endif
}

bool Platform::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"iconIndex", m_iconIndex);
	s >> MemberComposite< DeployTool >(L"deployToolWin32", m_deployToolWin32);
	s >> MemberComposite< DeployTool >(L"deployToolOsX", m_deployToolOsX);
	return true;
}

	}
}
