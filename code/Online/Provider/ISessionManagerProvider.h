#ifndef traktor_online_ISessionManagerProvider_H
#define traktor_online_ISessionManagerProvider_H

#include "Core/Object.h"

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

class IAchievementsProvider;
class ILeaderboardsProvider;
class IMatchMakingProvider;
class ISaveDataProvider;
class IStatisticsProvider;
class IUserProvider;

class T_DLLCLASS ISessionManagerProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool update() = 0;

	virtual std::wstring getLanguageCode() const = 0;

	virtual bool isConnected() const = 0;

	virtual bool requireUserAttention() const = 0;

	virtual uint64_t getCurrentUserHandle() const = 0;

	virtual bool haveP2PData() const = 0;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const = 0;

	virtual IAchievementsProvider* getAchievements() const = 0;

	virtual ILeaderboardsProvider* getLeaderboards() const = 0;

	virtual IMatchMakingProvider* getMatchMaking() const = 0;

	virtual ISaveDataProvider* getSaveData() const = 0;

	virtual IStatisticsProvider* getStatistics() const = 0;

	virtual IUserProvider* getUser() const = 0;
};

	}
}

#endif	// traktor_online_ISessionManagerProvider_H
