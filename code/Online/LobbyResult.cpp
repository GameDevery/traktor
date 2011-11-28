#include "Online/LobbyResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LobbyResult", LobbyResult, Result)

void LobbyResult::succeed(ILobby* lobby)
{
	m_lobby = lobby;
	Result::succeed();
}

ILobby* LobbyResult::get() const
{
	return m_lobby;
}

	}
}
