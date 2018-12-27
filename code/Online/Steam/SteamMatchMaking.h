/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_SteamMatchMaking_H
#define traktor_online_SteamMatchMaking_H

#include <steam/steam_api.h>
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamMatchMaking : public IMatchMakingProvider
{
	T_RTTI_CLASS;

public:
	SteamMatchMaking(SteamSessionManager* sessionManager);

	// Lobby

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles) override final;

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle) override final;

	virtual bool acceptLobby(uint64_t& outLobbyHandle) override final;

	virtual bool joinLobby(uint64_t lobbyHandle) override final;

	virtual bool leaveLobby(uint64_t lobbyHandle) override final;

	virtual bool setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles) override final;

	virtual bool getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle) override final;

	virtual bool setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const override final;

	virtual bool getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const override final;

	// Party

	virtual bool createParty(uint64_t& outPartyHandle) override final;

	virtual bool acceptParty(uint64_t& outPartyHandle) override final;

	virtual bool joinParty(uint64_t partyHandle) override final;

	virtual bool leaveParty(uint64_t partyHandle) override final;

	virtual bool setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles) override final;

	virtual bool getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const override final;

	virtual bool inviteToParty(uint64_t partyHandle, uint64_t userHandle) override final;

private:
	SteamSessionManager* m_sessionManager;
	std::vector< uint64_t >* m_outLobbies;
	uint64_t* m_outLobbyOrParty;
	uint64_t m_acceptedLobbyInvite;
	uint64_t m_acceptedPartyInvite;
	uint64_t m_joinedLobby;
	uint64_t m_joinedParty;
	bool m_joinResult;
	std::vector< uint64_t > m_lobbyParticipants;
	std::vector< uint64_t > m_partyParticipants;
	CCallResult< SteamMatchMaking, LobbyMatchList_t > m_callbackLobbyMatch;
	CCallResult< SteamMatchMaking, LobbyCreated_t > m_callbackLobbyCreated;
	CCallResult< SteamMatchMaking, LobbyEnter_t > m_callbackLobbyEnter;

	void updateLobbyParticipants();

	void updatePartyParticipants();

	void OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure);

	void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);

	void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure);

	STEAM_CALLBACK(SteamMatchMaking, OnGameLobbyJoinRequested, GameLobbyJoinRequested_t, m_callbackGameLobbyJoinRequested);

	STEAM_CALLBACK(SteamMatchMaking, OnLobbyChatUpdate, LobbyChatUpdate_t, m_callbackChatDataUpdate);
};

	}
}

#endif	// traktor_online_SteamMatchMaking_H
