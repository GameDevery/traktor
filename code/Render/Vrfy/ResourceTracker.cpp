/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/ResourceTracker.h"

#include "Core/Debug/CallStack.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"

namespace traktor::render
{

void ResourceTracker::add(const Object* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto& d = m_data[resource];
	getCallStack(16, d.callstack, 2);
}

void ResourceTracker::remove(const Object* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = m_data.find(resource);
	if (it != m_data.end())
		m_data.erase(it);
}

void ResourceTracker::alive()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (const auto& it : m_data)
	{
		log::info << L"Resource \"" << type_name(it.first) << L"\" (" << it.first->getReferenceCount() << L"):" << Endl;
		for (int32_t i = 0; i < 16; ++i)
		{
			std::wstring symbol;
			getSymbolFromAddress(it.second.callstack[i], symbol);

			std::wstring fileName;
			int32_t line;
			getSourceFromAddress(it.second.callstack[i], fileName, line);

			log::info << L"\t" << str(L"0x%016x", it.second.callstack[i]) << L"    " << symbol << L"    " << fileName << L"(" << line << L")" << Endl;
		}
	}
}

}
