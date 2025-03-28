/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Online/Local/LocalAchievements.h"
#include "Online/Local/LocalGameConfiguration.h"
#include "Online/Local/LocalLeaderboards.h"
#include "Online/Local/LocalMatchMaking.h"
#include "Online/Local/LocalSaveData.h"
#include "Online/Local/LocalStatistics.h"
#include "Online/Local/LocalSessionManager.h"
#include "Online/Local/LocalUser.h"
#include "Online/Local/LocalVideoSharing.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.LocalSessionManager", 0, LocalSessionManager, ISessionManagerProvider)

bool LocalSessionManager::create(const IGameConfiguration* configuration)
{
	const LocalGameConfiguration* gc = dynamic_type_cast< const LocalGameConfiguration* >(configuration);
	if (!gc)
		return false;

#if defined(__IOS__) || defined(__ANDROID__)
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/" + std::wstring(gc->m_dbName) + L".db";
#else
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/" + std::wstring(gc->m_dbName) + L".db";
#endif

	m_db = new sql::ConnectionSqlite3();
	if (!m_db->connect(L"fileName=" + dbPath))
	{
		log::error << L"Local session create failed; Unable to connect to local session database." << Endl;
		return false;
	}

	if (!m_db->tableExists(L"Achievements"))
	{
		if (m_db->executeUpdate(L"create table Achievements (id varchar(64) primary key, reward integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_achievementIds.begin(); i != gc->m_achievementIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_achievementIds.begin(); i != gc->m_achievementIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Achievements where id='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + *i + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"Leaderboards"))
	{
		if (m_db->executeUpdate(L"create table Leaderboards (id integer primary key, name varchar(64), score integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_leaderboardIds.begin(); i != gc->m_leaderboardIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_leaderboardIds.begin(); i != gc->m_leaderboardIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Leaderboards where name='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + *i + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"SaveData"))
	{
		if (m_db->executeUpdate(L"create table SaveData (id varchar(64) primary key, attachment varchar(4096))") < 0)
			return false;
	}

	if (!m_db->tableExists(L"Statistics"))
	{
		if (m_db->executeUpdate(L"create table Statistics (id varchar(64) primary key, value integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_statsIds.begin(); i != gc->m_statsIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_statsIds.begin(); i != gc->m_statsIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Statistics where id='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + *i + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"DLC"))
	{
		if (m_db->executeUpdate(L"create table DLC (id varchar(64) primary key, value boolean)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_dlcIds.begin(); i != gc->m_dlcIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into DLC (id, value) values ('" + *i + L"', false)") < 0)
				return false;
		}
	}

	m_achievements = new LocalAchievements(m_db);
	m_leaderboards = new LocalLeaderboards(m_db);
	m_matchMaking = new LocalMatchMaking();
	m_saveData = new LocalSaveData(m_db);
	m_statistics = new LocalStatistics(m_db);
	m_user = new LocalUser();
	m_videoSharing = new LocalVideoSharing();

	return true;
}

void LocalSessionManager::destroy()
{
	m_user = nullptr;
	m_statistics = nullptr;
	m_saveData = nullptr;
	m_matchMaking = nullptr;
	m_leaderboards = nullptr;
	m_achievements = nullptr;

	if (m_db)
	{
		m_db->disconnect();
		m_db = nullptr;
	}
}

bool LocalSessionManager::update()
{
	return true;
}

std::wstring LocalSessionManager::getLanguageCode() const
{
	return L"";
}

bool LocalSessionManager::isConnected() const
{
	return true;
}

bool LocalSessionManager::requireFullScreen() const
{
	return false;
}

bool LocalSessionManager::requireUserAttention() const
{
	return false;
}

bool LocalSessionManager::haveDLC(const std::wstring& id) const
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select value from DLC where id='" + id + L"'");
	if (!rs || !rs->next())
		return false;

	return rs->getInt32(L"value") > 0;
}

bool LocalSessionManager::buyDLC(const std::wstring& id) const
{
	if (m_db->executeUpdate(L"update DLC set value=true where id='" + id + L"'"))
		return true;
	else
		return false;
}

bool LocalSessionManager::navigateUrl(const net::Url& url) const
{
	return false;
}

uint64_t LocalSessionManager::getCurrentUserHandle() const
{
	return 0;
}

bool LocalSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	return true;
}

bool LocalSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
{
	return false;
}

bool LocalSessionManager::haveP2PData() const
{
	return false;
}

uint32_t LocalSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

uint32_t LocalSessionManager::getCurrentGameCount() const
{
	return 1;
}

IAchievementsProvider* LocalSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* LocalSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* LocalSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* LocalSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* LocalSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* LocalSessionManager::getUser() const
{
	return m_user;
}

IVideoSharingProvider* LocalSessionManager::getVideoSharing() const
{
	return m_videoSharing;
}

IVoiceChatProvider* LocalSessionManager::getVoiceChat() const
{
	return nullptr;
}

}
