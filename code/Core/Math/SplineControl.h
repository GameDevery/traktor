/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Clamp time.
 * \ingroup Core
 */
struct ClampTime
{
	static float t(float Tat, float Tfirst, float Tlast, float Tend)
	{
		float T = Tat;
		if (T < Tfirst)
			return Tfirst;
		else if (T > Tlast)
			return Tlast;
		else
			return T;
	}

	static int32_t index(int32_t i, int last)
	{
		if (i < 0)
			return 0;
		else if (i > last)
			return last;
		else
			return i;
	}
};

/*! Wrap time.
 * \ingroup Core
 */
struct WrapTime
{
	static float t(float Tat, float Tfirst, float Tlast, float Tend)
	{
		float T = Tat;
		float range = Tend - Tfirst;
		while (T < Tfirst)
			T += range;
		while (T > Tend + Tfirst)
			T -= range;
		return T;
	}

	static int32_t index(int32_t i, int32_t last)
	{
		while (i < 0)
			i += last + 1;
		while (i > last)
			i -= last + 1;
		return i;
	}
};

}

