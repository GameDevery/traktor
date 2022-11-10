/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Tracker/IEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS GotoEvent : public IEvent
{
	T_RTTI_CLASS;

public:
	GotoEvent(int32_t pattern, int32_t row);

	virtual bool execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const override final;

private:
	int32_t m_pattern;
	int32_t m_row;
};

	}
}
