/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

int32_t T_DLLCLASS getSystemDPI();

inline int32_t dpi96(int32_t referenceMeasure)
{
	return (getSystemDPI() * referenceMeasure) / 96;
}

inline int32_t invdpi96(int32_t measure)
{
	return (measure * 96) / getSystemDPI();
}

}