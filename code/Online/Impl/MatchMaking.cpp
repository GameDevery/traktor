/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Impl/Lobby.h"
#include "Online/Impl/MatchMaking.h"
#include "Online/Impl/Party.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskCreateLobby.h"
#include "Online/Impl/Tasks/TaskCreateParty.h"
#include "Online/Impl/Tasks/TaskFindMatchingLobbies.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.MatchMaking", MatchMaking, IMatchMaking)

bool MatchMaking::ready() const
{
	return true;
}

Ref< LobbyArrayResult > MatchMaking::findMatchingLobbies(const LobbyFilter* filter)
{
	Ref< LobbyArrayResult > result = new LobbyArrayResult();
	if (m_taskQueue->add(new TaskFindMatchingLobbies(
		m_matchMakingProvider,
		m_userCache,
		filter,
		result
	)))
		return result;
	else
		return 0;
}

Ref< LobbyResult > MatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access)
{
	Ref< LobbyResult > result = new LobbyResult();
	if (m_taskQueue->add(new TaskCreateLobby(
		m_matchMakingProvider,
		m_userCache,
		maxUsers,
		access,
		result
	)))
		return result;
	else
		return 0;
}

Ref< ILobby > MatchMaking::acceptLobby()
{
	uint64_t lobbyHandle;
	if (m_matchMakingProvider->acceptLobby(lobbyHandle))
		return new Lobby(m_matchMakingProvider, m_userCache, m_taskQueue, lobbyHandle);
	else
		return 0;
}

Ref< PartyResult > MatchMaking::createParty()
{
	Ref< PartyResult > result = new PartyResult();
	if (m_taskQueue->add(new TaskCreateParty(
		m_matchMakingProvider,
		m_userCache,
		result
	)))
		return result;
	else
		return 0;
}

Ref< IParty > MatchMaking::acceptParty()
{
	uint64_t lobbyHandle;
	if (m_matchMakingProvider->acceptParty(lobbyHandle))
		return new Party(m_matchMakingProvider, m_userCache, m_taskQueue, lobbyHandle);
	else
		return 0;
}

MatchMaking::MatchMaking(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_taskQueue(taskQueue)
{
}

	}
}
