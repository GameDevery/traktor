/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Render/Vulkan/Private/VertexAttributes.h"

namespace traktor::render
{

std::wstring VertexAttributes::getName(DataUsage usage, int32_t index)
{
	const wchar_t* s[] =
	{
		L"Position",
		L"Normal",
		L"Tangent",
		L"Binormal",
		L"Color",
		L"Custom",
		L""
	};
	return str(L"in_%s%d", s[(int32_t)usage], index);
}

int32_t VertexAttributes::getLocation(DataUsage usage, int32_t index)
{
	const int32_t base[] =
	{
		0,
		1,
		2,
		3,
		4,
		5,
		16
	};
	const int32_t location = base[(int32_t)usage] + index;
	return (location < base[(int32_t)usage + 1]) ? location : -1;
}

}
