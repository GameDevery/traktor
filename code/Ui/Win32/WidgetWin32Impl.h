#ifndef traktor_ui_WidgetWin32Impl_H
#define traktor_ui_WidgetWin32Impl_H

#include <map>
#include "Core/Misc/TString.h"
#include "Core/Misc/AutoPtr.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/ShowEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/MoveEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/FileDropEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Win32/Window.h"
#include "Ui/Win32/CanvasGdiWin32.h"
#if defined(T_USE_GDI_PLUS)
#	include "Ui/Win32/CanvasGdiPlusWin32.h"
#endif
#include "Ui/Win32/SmartHandle.h"
#include "Ui/Win32/UtilitiesWin32.h"

extern HINSTANCE g_hInstance;

namespace traktor
{
	namespace ui
	{

class EventSubject;
class ICanvas;

template < typename ControlType >
class WidgetWin32Impl : public ControlType
{
public:
	WidgetWin32Impl(EventSubject* owner)
	:	m_owner(owner)
	,	m_doubleBuffer(false)
	,	m_canvasImpl(0)
	{
	}

	virtual ~WidgetWin32Impl()
	{
		delete m_canvasImpl;
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void setParent(IWidget* parent)
	{
		SetParent(m_hWnd, static_cast< HWND >(parent->getInternalHandle()));
	}

	virtual void setText(const std::wstring& text)
	{
		SetWindowText(m_hWnd, wstots(text).c_str());
	}

	virtual std::wstring getText() const
	{
		int length = GetWindowTextLength(m_hWnd);
		if (length <= 0)
			return L"";

		AutoArrayPtr< TCHAR > buffer(new TCHAR [length + 1]);
		GetWindowText(m_hWnd, buffer.ptr(), length + 1);

		return tstows(buffer.ptr());
	}

	virtual void setToolTipText(const std::wstring& text)
	{
	}

	virtual void setForeground()
	{
		SetForegroundWindow(m_hWnd);
	}

	virtual bool isForeground() const
	{
		return bool(GetForegroundWindow() == m_hWnd);
	}

	virtual void setVisible(bool visible)
	{
		if (visible ^ isVisible(false))
		{
			ShowWindow(m_hWnd, visible ? SW_SHOWNA : SW_HIDE);

			ShowEvent showEvent(m_owner, 0, visible);
			m_owner->raiseEvent(EiShow, &showEvent);
		}
	}

	virtual bool isVisible(bool includingParents) const
	{
		if (includingParents)
			return bool(IsWindowVisible(m_hWnd) != FALSE);
		else
			return bool((GetWindowLong(m_hWnd, GWL_STYLE) & WS_VISIBLE) == WS_VISIBLE);
	}

	virtual void setActive()
	{
		SetActiveWindow(m_hWnd);
	}

	virtual void setEnable(bool enable)
	{
		EnableWindow(m_hWnd, enable);
	}

	virtual bool isEnable() const
	{
		return bool(IsWindowEnabled(m_hWnd) != FALSE);
	}

	virtual bool hasFocus() const
	{
		return bool(GetFocus() == m_hWnd);
	}

	virtual bool containFocus() const
	{
		for (HWND hWndFocus = GetFocus(); hWndFocus != NULL; hWndFocus = GetParent(hWndFocus))
		{
			if (hWndFocus == m_hWnd)
				return true;
		}
		return false;
	}

	virtual void setFocus()
	{
		SetFocus(m_hWnd);
	}

	virtual bool hasCapture() const
	{
		return bool(GetCapture() == m_hWnd);
	}

	virtual void setCapture()
	{
		SetCapture(m_hWnd);
	}

	virtual void releaseCapture()
	{
		if (hasCapture())
			ReleaseCapture();
	}

	virtual void startTimer(int interval, int id)
	{
		SetTimer(m_hWnd, 1000 + id, interval, NULL);
		m_timers[id] = interval;
	}

	virtual void stopTimer(int id)
	{
		std::map< uint32_t, uint32_t >::iterator i = m_timers.find(id);
		if (i != m_timers.end())
		{
			KillTimer(m_hWnd, 1000 + id);
			m_timers.erase(i);
		}
	}

	virtual void setOutline(const Point* p, int np)
	{
#if !defined(WINCE)
		std::vector< POINT > pts(np);
		for (int i = 0; i < np; ++i)
		{
			pts[i].x = p[i].x;
			pts[i].y = p[i].y;
		}
		HRGN hRegion = CreatePolygonRgn(&pts[0], np, WINDING);
		SetWindowRgn(m_hWnd, hRegion, TRUE);
		DeleteObject(hRegion);
#endif
	}

	virtual void setRect(const Rect& rect)
	{
		SetWindowPos(
			m_hWnd,
			NULL,
			rect.left,
			rect.top,
			rect.getWidth(),
			rect.getHeight(),
			SWP_NOZORDER | SWP_NOACTIVATE
		);
	}

	virtual Rect getRect() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);

		HWND hParent = GetParent(m_hWnd);
		if (hParent)
		{
			RECT rcParent;
			GetWindowRect(hParent, &rcParent);
			OffsetRect(&rc, -rcParent.left, -rcParent.top);
		}

		return Rect(rc.left, rc.top, rc.right, rc.bottom);
	}

	virtual Rect getInnerRect() const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return Rect(rc.left, rc.top, rc.right, rc.bottom);
	}

	virtual Rect getNormalRect() const
	{
#if !defined(WINCE)
		WINDOWPLACEMENT wp;
		std::memset(&wp, 0, sizeof(wp));
		wp.length = sizeof(wp);
		GetWindowPlacement(m_hWnd, &wp);
		const RECT rc = wp.rcNormalPosition;
		return Rect(rc.left, rc.top, rc.right, rc.bottom);
#else
		return Rect(0, 0, 0, 0);
#endif
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		return m_canvasImpl->getTextExtent(m_hWnd, text);
	}

	virtual void setFont(const Font& font)
	{
		LOGFONT lf;

		std::memset(&lf, 0, sizeof(lf));
		lf.lfHeight = font.getSize();
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = font.isBold() ? FW_BOLD : FW_NORMAL;
		lf.lfItalic = font.isItalic() ? TRUE : FALSE;
		lf.lfUnderline = font.isUnderline() ? TRUE : FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy_s(lf.lfFaceName, LF_FACESIZE, wstots(font.getFace()).c_str());

		m_hFont = CreateFontIndirect(&lf);
		m_hWnd.setFont(m_hFont);
	}

	virtual Font getFont() const
	{
		LOGFONT lf;
		BOOL result = GetObject(m_hWnd.getFont(), sizeof(lf), &lf);
		T_ASSERT_M (result, L"Unable to get device font");

		return Font(
			tstows(lf.lfFaceName),
			lf.lfHeight,
			bool(lf.lfWeight == FW_BOLD),
			bool(lf.lfItalic == TRUE),
			bool(lf.lfUnderline == TRUE)
		);
	}

	virtual void setCursor(Cursor cursor)
	{
		HCURSOR hCursor = NULL;
		switch (cursor)
		{
		case CrArrow:
			hCursor = LoadCursor(NULL, IDC_ARROW);
			break;

		case CrArrowRight:
			break;

		case CrArrowWait:
			hCursor = LoadCursor(NULL, IDC_WAIT);
			break;

		case CrCross:
			hCursor = LoadCursor(NULL, IDC_CROSS);
			break;

		case CrHand:
			hCursor = LoadCursor(NULL, IDC_HAND);
			break;

		case CrIBeam:
			hCursor = LoadCursor(NULL, IDC_IBEAM);
			break;

		case CrSizeNESW:
			hCursor = LoadCursor(NULL, IDC_SIZENESW);
			break;

		case CrSizeNS:
			hCursor = LoadCursor(NULL, IDC_SIZENS);
			break;

		case CrSizeNWSE:
			hCursor = LoadCursor(NULL, IDC_SIZENWSE);
			break;

		case CrSizeWE:
			hCursor = LoadCursor(NULL, IDC_SIZEWE);
			break;

		case CrSizing:
			hCursor = LoadCursor(NULL, IDC_SIZEALL);
			break;

		case CrWait:
			hCursor = LoadCursor(NULL, IDC_WAIT);
			break;

		case CrNone:
		default:
			break;
		};
		SetCursor(hCursor);
	}
	
	virtual Point getMousePosition(bool relative) const
	{
		POINT pnt;
		GetCursorPos(&pnt);
		if (relative)
			ScreenToClient(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual Point screenToClient(const Point& pt) const
	{
		POINT pnt = { pt.x, pt.y };
		ScreenToClient(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		POINT pnt = { pt.x, pt.y };
		ClientToScreen(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual bool hitTest(const Point& pt) const
	{
		POINT pnt = { pt.x, pt.y };
		return bool(WindowFromPoint(pnt) == m_hWnd);
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects)
	{
		HDWP hdwp = BeginDeferWindowPos(int(childRects.size()));
		if (hdwp)
		{
			for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
			{
				hdwp = DeferWindowPos(
					hdwp,
					(HWND)i->widget->getInternalHandle(),
					NULL,
					i->rect.left,
					i->rect.top,
					i->rect.getWidth(),
					i->rect.getHeight(),
					SWP_NOZORDER | SWP_NOACTIVATE
				);
				if (!hdwp)
					break;
			}
			if (hdwp)
			{
				EndDeferWindowPos(hdwp);
				return;
			}
		}

		// If we reach this point there has been an error in the deferred stuff, fall back on old style.
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			SetWindowPos(
				(HWND)i->widget->getInternalHandle(),
				NULL,
				i->rect.left,
				i->rect.top,
				i->rect.getWidth(),
				i->rect.getHeight(),
				SWP_NOZORDER | SWP_NOACTIVATE
			);
		}
	}

	virtual Size getMinimumSize() const
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const
	{
		return Size(0, 0);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		if (rc)
		{
			RECT wrc;
			wrc.left = rc->left;
			wrc.top = rc->top;
			wrc.right = rc->right;
			wrc.bottom = rc->bottom;
			RedrawWindow(m_hWnd, &wrc, NULL, RDW_INVALIDATE | (immediate ? RDW_UPDATENOW : 0)); 
		}
		else
		{
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | (immediate ? RDW_UPDATENOW : 0)); 
		}
	}

	virtual void* getInternalHandle()
	{
		return static_cast< void* >((HWND)m_hWnd);
	}

	virtual void* getSystemHandle()
	{
		return static_cast< void* >((HWND)m_hWnd);
	}

protected:
	EventSubject* m_owner;
	mutable Window m_hWnd;
	bool m_doubleBuffer;
	CanvasWin32* m_canvasImpl;
	SmartFont m_hFont;
	std::map< uint32_t, uint32_t > m_timers;

	static
	void getNativeStyles(int style, UINT& nativeStyle, UINT& nativeStyleEx)
	{
		nativeStyle = 0;
		nativeStyleEx = 0;

		if (style & WsBorder)
			nativeStyle |= WS_BORDER;
		if (style & WsClientBorder)
			nativeStyleEx |= WS_EX_CLIENTEDGE;
		if (style & WsResizable)
			nativeStyle |= WS_THICKFRAME;
		if (style & WsSystemBox)
			nativeStyle |= WS_SYSMENU;
		if (style & WsMinimizeBox)
			nativeStyle |= WS_MINIMIZEBOX;
		if (style & WsMaximizeBox)
			nativeStyle |= WS_MAXIMIZEBOX;
		if (style & WsCaption)
			nativeStyle |= WS_CAPTION;
		if (style & WsTop)
			nativeStyleEx |= WS_EX_TOPMOST;
	}

	bool create(int style)
	{
		if (!m_hWnd)
			return false;

		if (style & WsDoubleBuffer)
			m_doubleBuffer = true;

#if defined(T_USE_GDI_PLUS)
		m_canvasImpl = new CanvasGdiPlusWin32();
#else
		m_canvasImpl = new CanvasGdiWin32();
#endif

		m_hWnd.registerMessageHandler(WM_CHAR,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventChar));
		m_hWnd.registerMessageHandler(WM_KEYDOWN,       new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventKeyDown));
		m_hWnd.registerMessageHandler(WM_KEYUP,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventKeyUp));
		m_hWnd.registerMessageHandler(WM_MOVE,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMove));
		m_hWnd.registerMessageHandler(WM_SIZE,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventSize));
		m_hWnd.registerMessageHandler(WM_LBUTTONDOWN,   new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDown));
		m_hWnd.registerMessageHandler(WM_LBUTTONUP,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonUp));
		m_hWnd.registerMessageHandler(WM_LBUTTONDBLCLK, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDblClk));
		m_hWnd.registerMessageHandler(WM_RBUTTONDOWN,   new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDown));
		m_hWnd.registerMessageHandler(WM_RBUTTONUP,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonUp));
		m_hWnd.registerMessageHandler(WM_RBUTTONDBLCLK, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDblClk));
		m_hWnd.registerMessageHandler(WM_MOUSEMOVE,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMouseMove));
		m_hWnd.registerMessageHandler(WM_MOUSEWHEEL,    new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMouseWheel));
		m_hWnd.registerMessageHandler(WM_SETFOCUS,      new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventFocus));
		m_hWnd.registerMessageHandler(WM_KILLFOCUS,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventFocus));
		m_hWnd.registerMessageHandler(WM_PAINT,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventPaint));
		m_hWnd.registerMessageHandler(WM_ERASEBKGND,    new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventEraseBkGnd));
		m_hWnd.registerMessageHandler(WM_TIMER,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventTimer));
#if !defined(WINCE)
		m_hWnd.registerMessageHandler(WM_DROPFILES,		new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventDropFiles));
#endif

		return true;
	}

	LRESULT eventChar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		KeyEvent k(m_owner, 0, translateKeyCode(int(wParam)), int(wParam), wchar_t(wParam));
		m_owner->raiseEvent(EiKey, &k);
		if (!k.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventKeyDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		KeyEvent k(m_owner, 0, translateKeyCode(int(wParam)), int(wParam), 0);
		m_owner->raiseEvent(EiKeyDown, &k);
		if (!k.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventKeyUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		KeyEvent k(m_owner, 0, translateKeyCode(int(wParam)), int(wParam), 0);
		m_owner->raiseEvent(EiKeyUp, &k);
		if (!k.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		MoveEvent m(m_owner, 0, Point(LOWORD(lParam), HIWORD(lParam)));
		m_owner->raiseEvent(EiMove, &m);
		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		SizeEvent s(m_owner, 0, Size(LOWORD(lParam), HIWORD(lParam)));
		m_owner->raiseEvent(EiSize, &s);
		if (!s.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
#if defined(WINCE)
		if (message == WM_LBUTTONDOWN && m_owner->hasEventHandler(EiButtonDown))
		{
			SHRGINFO shrg;
			memset(&shrg, 0, sizeof(shrg));
			shrg.cbSize = sizeof(shrg);
			shrg.hwndClient = hWnd;
			shrg.ptDown.x = GET_X_LPARAM(lParam);
			shrg.ptDown.y = GET_Y_LPARAM(lParam);
			shrg.dwFlags = SHRG_RETURNCMD;
			if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU)
			{
				// Simulate right button down as it's most likely used for context menus in applications.
				message = WM_RBUTTONDOWN;
			}
		}
#endif

		MouseEvent::Button button = MouseEvent::BtNone;
		switch (message)
		{
		case WM_LBUTTONDOWN:
			button = MouseEvent::BtLeft;
			break;
		case WM_MBUTTONDOWN:
			button = MouseEvent::BtMiddle;
			break;
		case WM_RBUTTONDOWN:
			button = MouseEvent::BtRight;
			break;
		}

		MouseEvent m(
			m_owner,
			0,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(EiButtonDown, &m);

		if (!m.consumed())
			outPass = true;

		return TRUE;
	}

	LRESULT eventButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		MouseEvent::Button button = MouseEvent::BtNone;
		switch (message)
		{
		case WM_LBUTTONUP:
			button = MouseEvent::BtLeft;
			break;
		case WM_MBUTTONUP:
			button = MouseEvent::BtMiddle;
			break;
		case WM_RBUTTONUP:
			button = MouseEvent::BtRight;
			break;
		}

		MouseEvent m(
			m_owner,
			0,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(EiButtonUp, &m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventButtonDblClk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		MouseEvent::Button button = MouseEvent::BtNone;
		switch (message)
		{
		case WM_LBUTTONDBLCLK:
			button = MouseEvent::BtLeft;
			break;
		case WM_MBUTTONDBLCLK:
			button = MouseEvent::BtMiddle;
			break;
		case WM_RBUTTONDBLCLK:
			button = MouseEvent::BtRight;
			break;
		}

		MouseEvent m(
			m_owner,
			0,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(EiDoubleClick, &m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
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
		m_owner->raiseEvent(EiMouseMove, &m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		int button = MouseEvent::BtNone;
		if (LOWORD(wParam) & MK_LBUTTON)
			button |= MouseEvent::BtLeft;
		if (LOWORD(wParam) & MK_MBUTTON)
			button |= MouseEvent::BtMiddle;
		if (LOWORD(wParam) & MK_RBUTTON)
			button |= MouseEvent::BtRight;

		MouseEvent m(
			m_owner,
			0,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),
			GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA
		);
		m_owner->raiseEvent(EiMouseWheel, &m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventFocus(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		FocusEvent focusEvent(m_owner, 0, bool(message == WM_SETFOCUS));
		m_owner->raiseEvent(EiFocus, &focusEvent);

		outPass = true;
		return TRUE;
	}

	LRESULT eventPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (m_owner->hasEventHandler(EiPaint) && m_canvasImpl)
		{
			if (m_canvasImpl->beginPaint(m_hWnd, m_doubleBuffer, NULL))
			{
				Canvas canvas(m_canvasImpl);
				PaintEvent p(
					m_owner,
					0,
					canvas,
					Rect()
				);
				m_owner->raiseEvent(EiPaint, &p);
				m_canvasImpl->endPaint(m_hWnd);
				outPass = !p.consumed();
			}
		}
		else
			outPass = true;

		return 0;
	}

	LRESULT eventEraseBkGnd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (m_owner->hasEventHandler(EiPaint))
		{
			// Have paint event handler; return zero to indicate we didn't erase the background.
			outPass = false;
			return 0;
		}

		// No paint event handler; feed erase event down the chain of message handlers.
		outPass = true;
		return 1;
	}

	LRESULT eventTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (!IsWindowEnabled(m_hWnd))
			return TRUE;

		// Disable all timers until event is processed.
		for (std::map< uint32_t, uint32_t >::iterator i = m_timers.begin(); i != m_timers.end(); ++i)
			KillTimer(m_hWnd, i->first + 1000);

		CommandEvent c(m_owner, 0);
		m_owner->raiseEvent(EiTimer, &c);
		if (!c.consumed())
			outPass = true;

		// Restart all timers again.
		for (std::map< uint32_t, uint32_t >::iterator i = m_timers.begin(); i != m_timers.end(); ++i)
			SetTimer(m_hWnd, i->first + 1000, i->second, NULL);

		return TRUE;
	}

#if !defined(WINCE)
	LRESULT eventDropFiles(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		HDROP hDrop = (HDROP)wParam;

		UINT fileCount = DragQueryFile(hDrop, ~0UL, NULL, 0);
		if (!fileCount)
		{
			outPass = true;
			return TRUE;
		}

		std::vector< Path > files;
		for (UINT i = 0; i < fileCount; ++i)
		{
			TCHAR fileName[_MAX_PATH];
			DragQueryFile(hDrop, i, fileName, sizeof_array(fileName));
			files.push_back(tstows(fileName));
		}

		FileDropEvent e(m_owner, 0, files);
		m_owner->raiseEvent(EiFileDrop, &e);
		if (!e.consumed())
		{
			outPass = true;
			return TRUE;
		}

		DragFinish(hDrop);
		return FALSE;
	}
#endif
};

	}
}

#endif	// traktor_ui_WidgetWin32Impl_H
