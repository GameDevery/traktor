/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Result.h"
#include "Online/Impl/Tasks/TaskStatistics.h"
#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskStatistics", TaskStatistics, ITask)

TaskStatistics::TaskStatistics(
	IStatisticsProvider* provider,
	const std::wstring& statId,
	int32_t value,
	Result* result
)
:	m_provider(provider)
,	m_statId(statId)
,	m_value(value)
,	m_result(result)
{
}

void TaskStatistics::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	if (m_provider->set(
		m_statId,
		m_value
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
