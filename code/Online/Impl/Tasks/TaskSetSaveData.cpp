/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/AttachmentResult.h"
#include "Online/Impl/Tasks/TaskSetSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskSetSaveData", TaskSetSaveData, ITask)

TaskSetSaveData::TaskSetSaveData(
	ISaveDataProvider* provider,
	const std::wstring& saveDataId,
	const SaveDataDesc& saveDataDesc,
	const ISerializable* attachment,
	bool replace,
	Result* result
)
:	m_provider(provider)
,	m_saveDataId(saveDataId)
,	m_saveDataDesc(saveDataDesc)
,	m_attachment(attachment)
,	m_replace(replace)
,	m_result(result)
{
}

void TaskSetSaveData::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	if (m_provider->set(
		m_saveDataId,
		m_saveDataDesc,
		m_attachment,
		m_replace
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
