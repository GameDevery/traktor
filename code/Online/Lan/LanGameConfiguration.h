/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanGameConfiguration_H
#define traktor_online_LanGameConfiguration_H

#include <list>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LanGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	LanGameConfiguration();

	virtual void serialize(ISerializer& s) override final;
};

	}
}

#endif	// traktor_online_LanGameConfiguration_H
