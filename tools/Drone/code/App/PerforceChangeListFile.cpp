#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberEnum.h>
#include "App/PerforceChangeListFile.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.PerforceChangeListFile", PerforceChangeListFile, Serializable)

PerforceChangeListFile::PerforceChangeListFile()
:	m_action(AtNotOpened)
{
}

void PerforceChangeListFile::setDepotPath(const std::wstring& depotPath)
{
	m_depotPath = depotPath;
}

const std::wstring& PerforceChangeListFile::getDepotPath() const
{
	return m_depotPath;
}

void PerforceChangeListFile::setLocalPath(const std::wstring& localPath)
{
	m_localPath = localPath;
}

const std::wstring& PerforceChangeListFile::getLocalPath() const
{
	return m_localPath;
}

void PerforceChangeListFile::setAction(PerforceAction action)
{
	m_action = action;
}

PerforceAction PerforceChangeListFile::getAction() const
{
	return m_action;
}

bool PerforceChangeListFile::serialize(Serializer& s)
{
	const MemberEnum< PerforceAction >::Key c_PerforceAction_Keys[] =
	{
		{ L"AtNotOpened", AtNotOpened },
		{ L"AtAdd", AtAdd },
		{ L"AtEdit", AtEdit },
		{ L"AtDelete", AtDelete },
		{ 0 }
	};

	s >> Member< std::wstring >(L"depotPath", m_depotPath);
	s >> Member< std::wstring >(L"localPath", m_localPath);
	s >> MemberEnum< PerforceAction >(L"action", m_action, c_PerforceAction_Keys);

	return true;
}

	}
}
