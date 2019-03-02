#pragma once

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class UserArrayResult;
class UserCache;

class TaskGetLobbyParticipants : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetLobbyParticipants(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint64_t lobbyHandle,
		UserArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_lobbyHandle;
	Ref< UserArrayResult > m_result;
};

	}
}

