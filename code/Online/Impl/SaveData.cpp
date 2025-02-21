/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "Online/AttachmentResult.h"
#include "Online/Impl/SaveData.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskEnumSaveData.h"
#include "Online/Impl/Tasks/TaskGetSaveData.h"
#include "Online/Impl/Tasks/TaskRemoveSaveData.h"
#include "Online/Impl/Tasks/TaskSetSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SaveData", SaveData, ISaveData)

bool SaveData::ready() const
{
	return m_ready;
}

bool SaveData::enumerate(std::set< std::wstring >& outSaveDataIds) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outSaveDataIds = m_saveDataIds;
	return true;
}

Ref< AttachmentResult > SaveData::get(const std::wstring& saveDataId) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Ref< AttachmentResult > result = new AttachmentResult();
	if (m_taskQueue->add(new TaskGetSaveData(
		m_provider,
		saveDataId,
		result
	)))
		return result;
	else
		return 0;
}

Ref< ISerializable > SaveData::getNow(const std::wstring& saveDataId) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< ISerializable > attachment;
	if (m_provider->get(saveDataId, attachment))
		return attachment;
	else
		return 0;
}

Ref< Result > SaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskSetSaveData(
		m_provider,
		saveDataId,
		saveDataDesc,
		attachment,
		replace,
		result
	)))
		return result;
	else
		return 0;
}

bool SaveData::setNow(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_provider->set(saveDataId, saveDataDesc, attachment, replace);
}

Ref< Result > SaveData::remove(const std::wstring& saveDataId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskRemoveSaveData(m_provider, saveDataId, result)))
		return result;
	else
		return 0;
}

SaveData::SaveData(ISaveDataProvider* provider, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
,	m_ready(false)
{
}

void SaveData::enqueueEnumeration()
{
	m_taskQueue->add(new TaskEnumSaveData(
		m_provider,
		this,
		(TaskEnumSaveData::sink_method_t)&SaveData::callbackEnumSaveData
	));
}

void SaveData::callbackEnumSaveData(const std::set< std::wstring >& saveDataIds)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_saveDataIds = saveDataIds;
	m_ready = true;
}

	}
}
