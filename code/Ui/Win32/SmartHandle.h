/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Win32/Window.h"

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
struct GdiDeleteObjectPolicy
{
	static void deleteObject(HGDIOBJ h) { ::DeleteObject(h); }
};

/*!
 * \ingroup UIW32
 */
template < typename HandleType, typename DeletePolicy >
class SmartHandle
{
public:
	SmartHandle() = default;

	SmartHandle(HandleType h)
		: m_object(h)
	{
	}

	~SmartHandle()
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
	}

	SmartHandle& operator=(HandleType h)
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
		m_object = h;
		return *this;
	}

	SmartHandle& operator=(SmartHandle& sh)
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
		m_object = sh.m_object;
		sh.m_object = 0;
		return *this;
	}

	HandleType getHandle() const
	{
		return m_object;
	}

	operator HandleType()
	{
		return m_object;
	}

private:
	HandleType m_object = 0;
};

typedef SmartHandle< HFONT, GdiDeleteObjectPolicy > SmartFont;
typedef SmartHandle< HBRUSH, GdiDeleteObjectPolicy > SmartBrush;
typedef SmartHandle< HPEN, GdiDeleteObjectPolicy > SmartPen;

}
