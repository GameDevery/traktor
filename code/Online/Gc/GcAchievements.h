#pragma once

#include <list>
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class GcAchievementsImpl;

class GcAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	GcAchievements(const std::list< std::wstring >& achievementIds);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& achievementId, bool reward) T_OVERRIDE T_FINAL;

private:
	Ref< GcAchievementsImpl > m_impl;
};

	}
}

