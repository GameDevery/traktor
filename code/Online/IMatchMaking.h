/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/LobbyResult.h"
#include "Online/LobbyArrayResult.h"
#include "Online/PartyResult.h"
#include "Online/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class ILobby;
class IParty;
class LobbyFilter;

class T_DLLCLASS IMatchMaking : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) = 0;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers, LobbyAccess access) = 0;

	virtual Ref< ILobby > acceptLobby() = 0;

	virtual Ref< PartyResult > createParty() = 0;

	virtual Ref< IParty > acceptParty() = 0;
};

	}
}


