#include "Runtime/Editor/Deploy/Platform.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.runtime.Platform", 3, Platform, ISerializable)

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
#elif defined(__LINUX__) || defined(__RPI__)
	return m_deployToolLinux;
#else
	return m_deployToolWin64;
#endif
}

void Platform::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"iconIndex", m_iconIndex);

	if (s.getVersion() < 3)
	{
		DeployTool deployToolWin32;
		s >> MemberComposite< DeployTool >(L"deployToolWin32", deployToolWin32);
	}

	if (s.getVersion() >= 1)
		s >> MemberComposite< DeployTool >(L"deployToolWin64", m_deployToolWin64);

	s >> MemberComposite< DeployTool >(L"deployToolOsX", m_deployToolOsX);

	if (s.getVersion() >= 2)
		s >> MemberComposite< DeployTool >(L"deployToolLinux", m_deployToolLinux);
}

	}
}
