/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_SteamLeaderboards_H
#define traktor_online_SteamLeaderboards_H

#include <list>
#include <steam/steam_api.h>
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	SteamLeaderboards(SteamSessionManager* sessionManager, const std::list< std::wstring >& leaderboardIds);

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) override final;

	virtual bool create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard) override final;

	virtual bool set(uint64_t handle, int32_t score) override final;

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_leaderboardIds;
	CCallResult< SteamLeaderboards, LeaderboardScoreUploaded_t > m_callbackLeaderboardUploaded;
	CCallResult< SteamLeaderboards, LeaderboardScoresDownloaded_t > m_callbackLeaderboardDownloaded;
	bool m_uploadedScore;
	bool m_uploadedScoreSucceeded;
	bool m_downloadedScore;
	bool m_downloadedScoreSucceeded;
	std::vector< ScoreData >* m_outScores;

	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure);

	void OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamLeaderboards_H
