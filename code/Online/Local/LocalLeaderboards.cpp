#include "Online/Local/LocalLeaderboards.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalLeaderboards", LocalLeaderboards, ILeaderboardsProvider)

LocalLeaderboards::LocalLeaderboards(sql::IConnection* db)
:	m_db(db)
{
}

bool LocalLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select * from Leaderboards");
	if (!rs)
		return false;

	while (rs->next())
	{
		LeaderboardData data;
		data.handle = rs->getInt32(L"id");
		data.score = rs->getInt32(L"score");
		data.rank = 0;
		outLeaderboards.insert(std::make_pair(
			rs->getString(L"name"),
			data
		));
	}

	return true;
}

bool LocalLeaderboards::set(const uint64_t handle, int32_t score)
{
	if (m_db->executeUpdate(L"update Leaderboards set score=" + toString(score) + L" where id=" + toString(handle)) > 0)
		return true;
	else
		return false;
}

bool LocalLeaderboards::getScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	return false;
}

	}
}
