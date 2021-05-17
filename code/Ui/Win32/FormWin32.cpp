#include "Core/Log/Log.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/FormWin32.h"

namespace traktor
{
	namespace ui
	{

FormWin32::FormWin32(EventSubject* owner)
:	WidgetWin32Impl< IForm >(owner)
,	m_hWndLastFocus(NULL)
{
}

bool FormWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	DWORD nativeStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (style & WsResizable)
		nativeStyle |= WS_THICKFRAME;
	else
		nativeStyle |= WS_DLGFRAME;

	if (style & WsSystemBox)
		nativeStyle |= WS_SYSMENU | WS_CAPTION;
	if (style & WsMinimizeBox)
		nativeStyle |= WS_MINIMIZEBOX;
	if (style & WsMaximizeBox)
		nativeStyle |= WS_MAXIMIZEBOX;
	if (style & WsCaption)
		nativeStyle |= WS_CAPTION;

	if (parent)
		nativeStyle |= WS_CHILD;

	if (!m_hWnd.create(
		parent ? reinterpret_cast< HWND >(parent->getInternalHandle()) : NULL,
		_T("TraktorWin32Class"),
		wstots(text).c_str(),
		nativeStyle,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_CLOSE, new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventClose));
	m_hWnd.registerMessageHandler(WM_DESTROY, new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventDestroy));
	m_hWnd.registerMessageHandler(WM_ACTIVATE, new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventActivate));
	m_hWnd.registerMessageHandler(L"TaskbarButtonCreated", new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventTaskBarButtonCreated));

	m_ownCursor = true;
	return true;
}

void FormWin32::destroy()
{
	m_taskBarList.release();
	WidgetWin32Impl< IForm >::destroy();
}

void FormWin32::setVisible(bool visible)
{
	if (visible != isVisible())
	{
		ShowWindow(m_hWnd, visible ? SW_SHOW : SW_HIDE);

		ShowEvent showEvent(m_owner, visible);
		m_owner->raiseEvent(&showEvent);
	}
}

void FormWin32::setIcon(ISystemBitmap* icon)
{
	BitmapWin32* bm = static_cast< BitmapWin32* >(icon);
	HICON hIcon = bm->createIcon();
	m_hWnd.sendMessage(WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	m_hWnd.sendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

void FormWin32::maximize()
{
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);
}

void FormWin32::minimize()
{
	ShowWindow(m_hWnd, SW_MINIMIZE);
	UpdateWindow(m_hWnd);
}

void FormWin32::restore()
{
	ShowWindow(m_hWnd, SW_RESTORE);
	UpdateWindow(m_hWnd);
}

bool FormWin32::isMaximized() const
{
	BOOL zoomed = IsZoomed(m_hWnd);
	return bool(zoomed == TRUE);
}

bool FormWin32::isMinimized() const
{
	BOOL iconic = IsIconic(m_hWnd);
	return bool(iconic == TRUE);
}

void FormWin32::hideProgress()
{
	if (m_taskBarList)
		m_taskBarList->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
}

void FormWin32::showProgress(int32_t current, int32_t total)
{
	if (m_taskBarList)
	{
		m_taskBarList->SetProgressState(m_hWnd, TBPF_NORMAL);
		m_taskBarList->SetProgressValue(m_hWnd, current, total);
	}
}

LRESULT FormWin32::eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
	if (closeEvent.consumed() && closeEvent.cancelled())
		return TRUE;

	DestroyWindow(hWnd);
	return TRUE;
}

LRESULT FormWin32::eventDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	PostQuitMessage(0);
	return TRUE;
}

LRESULT FormWin32::eventActivate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
	{
		if (m_hWndLastFocus)
			SetFocus(m_hWndLastFocus);
	}
	else if (LOWORD(wParam) == WA_INACTIVE)
	{
		m_hWndLastFocus = GetFocus();
	}
	pass = false;
	return 0;
}

LRESULT FormWin32::eventTaskBarButtonCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_taskBarList.getAssign());
	if (m_taskBarList)
		m_taskBarList->SetProgressState(hWnd, TBPF_NOPROGRESS);
	return 0;
}

	}
}
