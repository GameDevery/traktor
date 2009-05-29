#include "Ui/Win32/ToolFormWin32.h"
#include "Ui/Events/CloseEvent.h"

namespace traktor
{
	namespace ui
	{

ToolFormWin32::ToolFormWin32(EventSubject* owner) :
	WidgetWin32Impl< IToolForm >(owner)
{
}

bool ToolFormWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (!m_hWnd.create(
		parent ? (HWND)parent->getInternalHandle() : NULL,
		_T("TraktorWin32Class"),
		wstots(text).c_str(),
#if !defined(WINCE)
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | nativeStyle,
#else
		nativeStyle,
#endif
		nativeStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

#if !defined(WINCE)
	m_hWnd.registerMessageHandler(WM_NCLBUTTONDOWN, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonDown));
	m_hWnd.registerMessageHandler(WM_NCLBUTTONUP, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonUp));
	m_hWnd.registerMessageHandler(WM_NCRBUTTONDOWN, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonDown));
	m_hWnd.registerMessageHandler(WM_NCRBUTTONUP, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonUp));
	m_hWnd.registerMessageHandler(WM_NCMOUSEMOVE, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcMouseMove));
#endif

	return true;
}

void ToolFormWin32::center()
{
	HWND hParent = GetParent(m_hWnd);
	if (!hParent)
		hParent = GetDesktopWindow();

	RECT rcParent;
	GetWindowRect(hParent, &rcParent);

	RECT rcTool;
	GetWindowRect(m_hWnd, &rcTool);

	POINT pntPos =
	{
		rcParent.left + (rcParent.right - rcParent.left - rcTool.right + rcTool.left) / 2,
		rcParent.top + (rcParent.bottom - rcParent.top - rcTool.bottom + rcTool.top) / 2
	};
	if (pntPos.x < 0)
		pntPos.x = 0;
	if (pntPos.y < 0)
		pntPos.y = 0;

	SetWindowPos(m_hWnd, NULL, pntPos.x, pntPos.y, 0, 0, SWP_NOSIZE);
}

#if !defined(WINCE)

LRESULT ToolFormWin32::eventNcButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	MouseEvent::Button button = MouseEvent::BtNone;
	switch (message)
	{
	case WM_NCLBUTTONDOWN:
		button = MouseEvent::BtLeft;
		break;
	case WM_NCRBUTTONDOWN:
		button = MouseEvent::BtRight;
		break;
	}

	MouseEvent m(
		m_owner,
		0,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(EiNcButtonDown, &m);

	if (!m.consumed())
		pass = true;

	return 0;
}

LRESULT ToolFormWin32::eventNcButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	MouseEvent::Button button = MouseEvent::BtNone;
	switch (message)
	{
	case WM_NCLBUTTONDOWN:
		button = MouseEvent::BtLeft;
		break;
	case WM_NCRBUTTONDOWN:
		button = MouseEvent::BtRight;
		break;
	}

	MouseEvent m(
		m_owner,
		0,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(EiNcButtonUp, &m);

	if (!m.consumed())
		pass = true;

	return 0;
}

LRESULT ToolFormWin32::eventNcMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	int button = MouseEvent::BtNone;
	if (wParam & MK_LBUTTON)
		button |= MouseEvent::BtLeft;
	if (wParam & MK_MBUTTON)
		button |= MouseEvent::BtMiddle;
	if (wParam & MK_RBUTTON)
		button |= MouseEvent::BtRight;

	MouseEvent m(
		m_owner,
		0,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(EiNcMouseMove, &m);

	if (!m.consumed())
		pass = true;

	return 0;
}

#endif	// !WINCE

	}
}
