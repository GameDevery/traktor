/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Local/LocalMatchMaking.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalMatchMaking", LocalMatchMaking, IMatchMakingProvider)

bool LocalMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	return false;
}

bool LocalMatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return true;
}

bool LocalMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return false;
}

bool LocalMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	return false;
}

bool LocalMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	return true;
}

bool LocalMatchMaking::setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	m_lobbyMeta[key] = value;
	return true;
}

bool LocalMatchMaking::getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_lobbyMeta.find(key);
	if (i == m_lobbyMeta.end())
		return false;

	outValue = i->second;
	return true;
}

bool LocalMatchMaking::setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	return true;
}

bool LocalMatchMaking::getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return true;
}

bool LocalMatchMaking::getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	return true;
}

bool LocalMatchMaking::getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle)
{
	return false;
}

bool LocalMatchMaking::setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	return false;
}

bool LocalMatchMaking::getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	outUserHandle = 0;
	return true;
}

bool LocalMatchMaking::createParty(uint64_t& outPartyHandle)
{
	return false;
}

bool LocalMatchMaking::acceptParty(uint64_t& outPartyHandle)
{
	return false;
}

bool LocalMatchMaking::joinParty(uint64_t partyHandle)
{
	return false;
}

bool LocalMatchMaking::leaveParty(uint64_t partyHandle)
{
	return false;
}

bool LocalMatchMaking::setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LocalMatchMaking::getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LocalMatchMaking::setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LocalMatchMaking::getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LocalMatchMaking::getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles)
{
	return false;
}

bool LocalMatchMaking::getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::inviteToParty(uint64_t partyHandle, uint64_t userHandle)
{
	return false;
}

}
