#ifndef traktor_online_ILeaderboardsProvider_H
#define traktor_online_ILeaderboardsProvider_H

#include <map>
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

class T_DLLCLASS ILeaderboardsProvider : public Object
{
	T_RTTI_CLASS;

public:
	struct LeaderboardData
	{
		uint64_t handle;
		int32_t score;
		uint32_t rank;
	};

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) = 0;

	virtual bool set(uint64_t handle, int32_t score) = 0;

	virtual bool getScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores) = 0;
};

	}
}

#endif	// traktor_online_ILeaderboardsProvider_H
