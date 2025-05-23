/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <Windows.h>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/Win32/Window.h"

extern HINSTANCE g_hInstance;

#define SET_WINDOW_LONG_PTR(a, b, c) SetWindowLongPtr(a, b, c)
#define GET_WINDOW_LONG_PTR(a, b) GetWindowLongPtr(a, b)

namespace traktor::ui
{
	namespace
	{

UINT getReflectedMessage(UINT message)
{
	switch (message)
	{
	case WM_COMMAND:
		return WM_REFLECTED_COMMAND;

	case WM_HSCROLL:
		return WM_REFLECTED_HSCROLL;

	case WM_VSCROLL:
		return WM_REFLECTED_VSCROLL;

	case WM_CTLCOLORSTATIC:
		return WM_REFLECTED_CTLCOLORSTATIC;

	case WM_CTLCOLOREDIT:
		return WM_REFLECTED_CTLCOLOREDIT;

	case WM_CTLCOLORBTN:
		return WM_REFLECTED_CTLCOLORBTN;

	case WM_CTLCOLORLISTBOX:
		return WM_REFLECTED_CTLCOLORLISTBOX;

	default:
		return 0;
	}
}

	}

Window::Window()
:	m_hWnd(0)
,	m_originalWndProc(nullptr)
{
}

Window::~Window()
{
	// Ensure message handlers are cleared before we remove our callback since
	// SetWindowLong seems to be called when removing callback.
	m_messageHandlers.clear();

	if (m_originalWndProc)
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_originalWndProc);

	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)0);

	DestroyWindow(m_hWnd);
}

bool Window::create(
	HWND hParentWnd,
	LPCTSTR className,
	LPCTSTR text,
	UINT style,
	UINT styleEx,
	int left,
	int top,
	int width,
	int height,
	int id,
	bool subClass
)
{
	T_ASSERT_M (!m_hWnd, L"Window already created");

	// In case we've got a parent and no position we need to ensure
	// window is created on the same desktop as the parent. This
	// is because we want the same DPI reported for child window as parent.
	if (hParentWnd != NULL && left == CW_USEDEFAULT && top == CW_USEDEFAULT)
	{
		RECT rcParent;
		GetWindowRect(hParentWnd, &rcParent);
		left = rcParent.left;
		top = rcParent.top;
	}

	m_hWnd = CreateWindowEx(
		styleEx,
		className,
		text,
		style,
		left,
		top,
		width,
		height,
		hParentWnd,
		(HMENU)id,
		g_hInstance,
		NULL
	);
	if (!m_hWnd)
	{
		log::error << L"Unable to create window" << Endl;
		return false;
	}

	if (_tcscmp(className, _T("TraktorDialogWin32Class")) != 0)
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	else
		SET_WINDOW_LONG_PTR(m_hWnd, DWLP_USER, (LONG_PTR)this);

	if (subClass)
	{
		m_originalWndProc = (WNDPROC)GET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC);
		SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndProcSubClass);
	}

	return true;
}

bool Window::subClass(HWND hWnd)
{
	T_ASSERT(!m_hWnd);

	m_hWnd = hWnd;
	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	m_originalWndProc = (WNDPROC)GET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC);
	SET_WINDOW_LONG_PTR(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndProcSubClass);

	return true;
}

int32_t Window::dpi() const
{
	HWND hWnd = m_hWnd;

	while (hWnd != NULL && (GetWindowLong(m_hWnd, GWL_STYLE) & WS_VISIBLE) != WS_VISIBLE)
		hWnd = GetParent(hWnd);

	if (hWnd == NULL)
		hWnd = m_hWnd;

	return GetDpiForWindow(hWnd);
}

LRESULT Window::sendMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{
	return SendMessage(m_hWnd, message, wParam, lParam);
}

Window::operator HWND () const
{
	return m_hWnd;
}

void Window::registerMessageHandler(UINT message, IMessageHandler* messageHandler)
{
	m_messageHandlers[message] = messageHandler;
}

void Window::registerMessageHandler(const wchar_t* message, IMessageHandler* messageHandler)
{
	UINT id = RegisterWindowMessage(message);
	registerMessageHandler(id, messageHandler);
}

bool Window::haveMessageHandler(UINT message) const
{
	return m_messageHandlers.find(message) != m_messageHandlers.end();
}

void Window::registerDefaultClass()
{
	WNDCLASS wc;

	std::memset(&wc, 0, sizeof(wc));
	wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = wndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	wc.hInstance     = g_hInstance;
	wc.hbrBackground = NULL;
	wc.hIcon         = LoadIcon(g_hInstance, _T("DEFAULTICON"));
	wc.hCursor       = NULL; // LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("TraktorWin32Class");

	if (!RegisterClass(&wc))
		log::error << L"Unable to register class \"TraktorWin32Class\"" << Endl;
}

void Window::unregisterDefaultClass()
{
	UnregisterClass(_T("TraktorWin32Class"), g_hInstance);
}

void Window::registerDialogClass()
{
	WNDCLASS wc;

	std::memset(&wc, 0, sizeof(wc));
	wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = dlgProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = DLGWINDOWEXTRA + sizeof(void*);
	wc.hInstance     = g_hInstance;
	wc.hbrBackground = NULL;
	wc.hIcon         = LoadIcon(g_hInstance, _T("DEFAULTICON"));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("TraktorDialogWin32Class");

	if (!RegisterClass(&wc))
		log::error << L"Unable to register class \"TraktorDialogWin32Class\"" << Endl;
}

void Window::unregisterDialogClass()
{
	UnregisterClass(_T("TraktorDialogWin32Class"), g_hInstance);
}

LRESULT Window::invokeMessageHandlers(HWND hWnd, DWORD dwIndex, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	LRESULT result = FALSE;

	Window* window = reinterpret_cast< Window* >(GET_WINDOW_LONG_PTR(hWnd, dwIndex));
	if (window)
	{
		Ref< IMessageHandler > messageHandler = window->m_messageHandlers[message];
		if (messageHandler)
		{
			pass = false;
			result = messageHandler->handle(hWnd, message, wParam, lParam, pass);
		}
		else
			pass = true;
	}
	else
		pass = true;

	return result;
}

LRESULT CALLBACK Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	UINT reflectedMessage;
	bool pass;

	// Lookup handler of issued message.
	result = invokeMessageHandlers(hWnd, GWLP_USERDATA, message, wParam, lParam, pass);
	if (!pass)
		return result;

	// Reflect messages sent to parents back to issuing child.
	if (message == WM_NOTIFY)
	{
		LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
		if (nmhdr && nmhdr->hwndFrom)
		{
			result = invokeMessageHandlers(nmhdr->hwndFrom, GWLP_USERDATA, WM_REFLECTED_NOTIFY, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if ((reflectedMessage = getReflectedMessage(message)) != 0)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, reflectedMessage, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}

	// Call default window procedure.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Window::dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	bool pass;

	// Lookup handler of issued message.
	result = invokeMessageHandlers(hWnd, DWLP_USER, message, wParam, lParam, pass);
	if (!pass)
		return result;

	// Reflect messages sent to parents back to issuing child.
	if (message == WM_COMMAND)
	{
		HWND hWndControl = NULL;
		if (HIWORD(wParam) == 0)
			hWndControl = GetDlgItem(hWnd, LOWORD(wParam));
		else
			hWndControl = (HWND)lParam;

		if (hWndControl)
		{
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, WM_REFLECTED_COMMAND, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_NOTIFY)
	{
		LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
		if (nmhdr && nmhdr->hwndFrom)
		{
			result = invokeMessageHandlers(nmhdr->hwndFrom, GWLP_USERDATA, WM_REFLECTED_NOTIFY, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}
	else if (message == WM_HSCROLL || message == WM_VSCROLL)
	{
		HWND hWndControl = (HWND)lParam;
		if (hWndControl)
		{
			UINT reflectMsg = (message == WM_HSCROLL) ? WM_REFLECTED_HSCROLL : WM_REFLECTED_VSCROLL;
			result = invokeMessageHandlers(hWndControl, GWLP_USERDATA, reflectMsg, wParam, lParam, pass);
			if (!pass)
				return result;
		}
	}

	// Call default window procedure; note cannot call DefDlgProc since
	// it will dead-lock application.
	return DefWindowProc(
		hWnd,
		message,
		wParam,
		lParam
	);
}

LRESULT CALLBACK Window::wndProcSubClass(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Lookup handler of issued message.
	Window* window = reinterpret_cast< Window* >(GET_WINDOW_LONG_PTR(hWnd, GWLP_USERDATA));
	if (window)
	{
		WNDPROC originalWndProc = window->m_originalWndProc;

		// Call custom message handler.
		IMessageHandler* messageHandler = window->m_messageHandlers[message];
		if (messageHandler)
		{
			bool pass = false;
			LRESULT result = messageHandler->handle(hWnd, message, wParam, lParam, pass);
			if (!pass)
				return result;
		}

		// Call original window procedure.
		if (originalWndProc && IsWindow(hWnd))
			return CallWindowProc(
				originalWndProc,
				hWnd,
				message,
				wParam,
				lParam
			);
	}

	// Unable to get original window procedure, call default window procedure.
	return DefWindowProc(
		hWnd,
		message,
		wParam,
		lParam
	);
}

}
