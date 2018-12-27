/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LocalAchievements_H
#define traktor_online_LocalAchievements_H

#include "Core/Ref.h"
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	LocalAchievements(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalAchievements_H
