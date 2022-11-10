/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCompare.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StreamCompare", StreamCompare, Object)

StreamCompare::StreamCompare(IStream* first, IStream* second)
:	m_first(first)
,	m_second(second)
{
}

bool StreamCompare::execute()
{
	int64_t firstPos = m_first->tell();
	int64_t secondPos = m_second->tell();

	uint8_t firstBuf[1024];
	uint8_t secondBuf[1024];

	bool result = true;

	for (;;)
	{
		int64_t r0 = m_first->read(firstBuf, sizeof(firstBuf));
		int64_t r1 = m_second->read(secondBuf, sizeof(secondBuf));

		if (r0 != r1)
		{
			result = false;
			break;
		}

		if (r0 <= 0)
			break;

		if (std::memcmp(firstBuf, secondBuf, r0) != 0)
		{
			result = false;
			break;
		}
	}

	m_first->seek(IStream::SeekSet, firstPos);
	m_second->seek(IStream::SeekSet, secondPos);

	return result;
}

}
