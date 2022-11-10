/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Impl/User.h"
#include "Online/Provider/IUserProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.User", User, IUser)

bool User::getName(std::wstring& outName) const
{
	return m_userProvider->getName(m_handle, outName);
}

Ref< drawing::Image > User::getImage() const
{
	return m_userProvider->getImage(m_handle);
}

uint64_t User::getGlobalId() const
{
	return m_handle;
}

bool User::isFriend() const
{
	return m_userProvider->isFriend(m_handle);
}

bool User::isMemberOfGroup(const std::wstring& groupName) const
{
	return m_userProvider->isMemberOfGroup(m_handle, groupName);
}

bool User::joinGroup(const std::wstring& groupName)
{
	return m_userProvider->joinGroup(m_handle, groupName);
}

bool User::invite()
{
	return m_userProvider->invite(m_handle);
}

bool User::setPresenceValue(const std::wstring& key, const std::wstring& value)
{
	return m_userProvider->setPresenceValue(m_handle, key, value);
}

bool User::getPresenceValue(const std::wstring& key, std::wstring& outValue) const
{
	return m_userProvider->getPresenceValue(m_handle, key, outValue);
}

void User::setP2PEnable(bool enable)
{
	m_userProvider->setP2PEnable(m_handle, enable);
}

bool User::isP2PAllowed() const
{
	return m_userProvider->isP2PAllowed(m_handle);
}

bool User::isP2PRelayed() const
{
	return m_userProvider->isP2PRelayed(m_handle);
}

bool User::sendP2PData(const void* data, size_t size, bool reliable)
{
	return m_userProvider->sendP2PData(m_handle, data, size, reliable);
}

User::User(IUserProvider* userProvider, uint64_t handle)
:	m_userProvider(userProvider)
,	m_handle(handle)
{
}

	}
}
