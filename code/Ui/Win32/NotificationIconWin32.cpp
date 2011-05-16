#include "Ui/Win32/NotificationIconWin32.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/MouseEvent.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

uint32_t s_taskbarCreated = 0;

		}

NotificationIconWin32::NotificationIconWin32(EventSubject* owner)
:	m_owner(owner)
{
}

bool NotificationIconWin32::create(const std::wstring& text, IBitmap* image)
{
	if (!m_hWnd.create(
		NULL,
		_T("TraktorWin32Class"),
		_T(""),
		0,
		0,
		0,
		0,
		0,
		0
	))
		return false;

	m_hWnd.registerMessageHandler(WM_USER + 1, new MethodMessageHandler< NotificationIconWin32 >(this, &NotificationIconWin32::eventNotification));

	std::memset(&m_nid, 0, sizeof(m_nid));
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = 1;
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_nid.uCallbackMessage = WM_USER + 1;
	m_nid.hIcon = reinterpret_cast< BitmapWin32* >(image)->createIcon();
	_tcscpy_s(m_nid.szTip, sizeof_array(m_nid.szTip), wstots(text).c_str());
	if (!Shell_NotifyIcon(NIM_ADD, &m_nid))
		return false;

	s_taskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
	m_hWnd.registerMessageHandler(s_taskbarCreated, new MethodMessageHandler< NotificationIconWin32 >(this, &NotificationIconWin32::eventTaskbarCreated));

	return true;
}

void NotificationIconWin32::destroy()
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	delete this;
}

void NotificationIconWin32::setImage(IBitmap* image)
{
	T_ASSERT (image);
	m_nid.hIcon = reinterpret_cast< BitmapWin32* >(image)->createIcon();
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

LRESULT NotificationIconWin32::eventNotification(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	POINT pnt;
	GetCursorPos(&pnt);

	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiDoubleClick, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONDBLCLK:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiDoubleClick, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiButtonDown, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiButtonDown, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_LBUTTONUP:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiButtonUp, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONUP:
		{
			MouseEvent m(
				m_owner,
				0,
				MouseEvent::BtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(EiButtonUp, &m);
			if (!m.consumed())
				pass = true;
		}
		break;

	default:
		pass = true;
	}
	return 0;
}

LRESULT NotificationIconWin32::eventTaskbarCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	// Task bar created; most possibly after a explorer crash.
	Shell_NotifyIcon(NIM_ADD, &m_nid);
	return 0;
}

	}
}
