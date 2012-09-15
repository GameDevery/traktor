#ifndef traktor_online_TaskGetScores_H
#define traktor_online_TaskGetScores_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ILeaderboardsProvider;
class ScoreArrayResult;
class UserCache;

class TaskGetScores : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetScores(
		ILeaderboardsProvider* leaderboardProvider,
		UserCache* userCache,
		uint64_t handle,
		int32_t from,
		int32_t to,
		ScoreArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< ILeaderboardsProvider > m_leaderboardProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_handle;
	int32_t m_from;
	int32_t m_to;
	Ref< ScoreArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskGetScores_H
