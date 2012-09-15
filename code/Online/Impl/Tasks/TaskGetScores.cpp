#include "Online/Score.h"
#include "Online/ScoreArrayResult.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
#include "Online/Impl/Tasks/TaskGetScores.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskGetScores", TaskGetScores, ITask)

TaskGetScores::TaskGetScores(
	ILeaderboardsProvider* leaderboardProvider,
	UserCache* userCache,
	uint64_t handle,
	int32_t from,
	int32_t to,
	ScoreArrayResult* result
)
:	m_leaderboardProvider(leaderboardProvider)
,	m_userCache(userCache)
,	m_handle(handle)
,	m_from(from)
,	m_to(to)
,	m_result(result)
{
}

void TaskGetScores::execute(TaskQueue* taskQueue)
{
	std::vector< std::pair< uint64_t, int32_t > > providerScores;
	RefArray< Score > scores;

	if (m_leaderboardProvider->getScores(m_handle, m_from, m_to, providerScores))
	{
		scores.reserve(providerScores.size());
		for (std::vector< std::pair< uint64_t, int32_t > >::const_iterator i = providerScores.begin(); i != providerScores.end(); ++i)
		{
			Ref< IUser > user = m_userCache->get(i->first);
			if (user)
				scores.push_back(new Score(user, i->second));
		}
		m_result->succeed(scores);
	}
	else
		m_result->fail();
}

	}
}
