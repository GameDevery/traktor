/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Win32/UserWidgetWin32.h"

namespace traktor::ui
{

UserWidgetWin32::UserWidgetWin32(EventSubject* owner)
:	WidgetWin32Impl< IUserWidget >(owner)
{
}

bool UserWidgetWin32::create(IWidget* parent, int style)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD dwStyleEx = WS_EX_CONTROLPARENT;

	if (style & WsBorder)
		dwStyle |= WS_BORDER;
	if (style & WsTabStop)
		dwStyle |= WS_TABSTOP;
	if (style & WsClientBorder)
		dwStyleEx |= WS_EX_CLIENTEDGE;
	if (style & WsAcceptFileDrop)
		dwStyleEx |= WS_EX_ACCEPTFILES;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("TraktorWin32Class"),
		_T(""),
		dwStyle,
		dwStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_LBUTTONDOWN, new MethodMessageHandler< UserWidgetWin32 >(this, &UserWidgetWin32::eventButtonDown));

	m_ownCursor = true;
	return true;
}

LRESULT UserWidgetWin32::eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	LRESULT result = WidgetWin32Impl::eventButtonDown(hWnd, message, wParam, lParam, pass);
	if (pass)
	{
		// If message wasn't handled we set focus to our user widget.
		SetFocus(m_hWnd);
	}
	return result;
}

}
