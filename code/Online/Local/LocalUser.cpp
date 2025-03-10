/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System/OS.h"
#include "Online/Local/LocalUser.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalUser", LocalUser, IUserProvider)

bool LocalUser::getName(uint64_t userHandle, std::wstring& outName)
{
	outName = OS::getInstance().getCurrentUser();
	return true;
}

Ref< drawing::Image > LocalUser::getImage(uint64_t userHandle) const
{
	return nullptr;
}

bool LocalUser::isFriend(uint64_t userHandle)
{
	return true;
}

bool LocalUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool LocalUser::joinGroup(uint64_t userHandle, const std::wstring& groupName)
{
	return false;
}

bool LocalUser::invite(uint64_t userHandle)
{
	return false;
}

bool LocalUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LocalUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

void LocalUser::setP2PEnable(uint64_t userHandle, bool enable)
{
}

bool LocalUser::isP2PAllowed(uint64_t userHandle) const
{
	return false;
}

bool LocalUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool LocalUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	return false;
}

}
