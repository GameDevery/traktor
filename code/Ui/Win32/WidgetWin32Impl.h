#pragma once

#include "Core/Misc/TString.h"
#include "Core/Misc/AutoPtr.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Win32/Window.h"
#include "Ui/Win32/SmartHandle.h"
#include "Ui/Win32/UtilitiesWin32.h"

#if defined(T_USE_DIRECT2D)
#	include "Ui/Win32/CanvasDirect2DWin32.h"
#endif
#if defined(T_USE_GDI_PLUS)
#	include "Ui/Win32/CanvasGdiPlusWin32.h"
#endif
#if defined(T_USE_GDI)
#	include "Ui/Win32/CanvasGdiWin32.h"
#endif

extern HINSTANCE g_hInstance;

namespace traktor
{
	namespace ui
	{

class EventSubject;
class ICanvas;

template < typename ControlType >
class WidgetWin32Impl
:	public ControlType
,	public IFontMetric
{
public:
	WidgetWin32Impl(EventSubject* owner)
	:	m_owner(owner)
	,	m_doubleBuffer(false)
	,	m_canvasImpl(nullptr)
	,	m_hCursor(NULL)
	,	m_ownCursor(false)
	,	m_tracking(false)
	,	m_interval(-1)
	{
	}

	virtual ~WidgetWin32Impl()
	{
		delete m_canvasImpl;
	}

	virtual void destroy() override
	{
		KillTimer(m_hWnd, 1000);
		delete this;
	}

	virtual void setParent(IWidget* parent) override
	{
		SetParent(m_hWnd, static_cast< HWND >(parent->getInternalHandle()));
	}

	virtual void setText(const std::wstring& text) override
	{
		SetWindowText(m_hWnd, wstots(text).c_str());
	}

	virtual std::wstring getText() const override
	{
		int length = GetWindowTextLength(m_hWnd);
		if (length <= 0)
			return L"";

		AutoArrayPtr< TCHAR > buffer(new TCHAR [length + 1]);
		GetWindowText(m_hWnd, buffer.ptr(), length + 1);

		return tstows(buffer.ptr());
	}

	virtual void setForeground() override
	{
		SetForegroundWindow(m_hWnd);
	}

	virtual bool isForeground() const override
	{
		return bool(GetForegroundWindow() == m_hWnd);
	}

	virtual void setVisible(bool visible) override
	{
		if (visible != isVisible())
		{
			ShowWindow(m_hWnd, visible ? SW_SHOWNA : SW_HIDE);

			ShowEvent showEvent(m_owner, visible);
			m_owner->raiseEvent(&showEvent);
		}
	}

	virtual bool isVisible() const override
	{
		return bool((GetWindowLong(m_hWnd, GWL_STYLE) & WS_VISIBLE) == WS_VISIBLE);
	}

	virtual void setEnable(bool enable) override
	{
		EnableWindow(m_hWnd, enable);
	}

	virtual bool isEnable() const override
	{
		return bool(IsWindowEnabled(m_hWnd) != FALSE);
	}

	virtual bool hasFocus() const override
	{
		return bool(GetFocus() == m_hWnd);
	}

	virtual void setFocus() override
	{
		SetFocus(m_hWnd);
	}

	virtual bool hasCapture() const override
	{
		return bool(GetCapture() == m_hWnd);
	}

	virtual void setCapture() override
	{
		SetCapture(m_hWnd);
	}

	virtual void releaseCapture() override
	{
		if (hasCapture())
			ReleaseCapture();
	}

	virtual void startTimer(int interval) override
	{
		SetTimer(m_hWnd, 1000, interval, NULL);
		m_interval = interval;
	}

	virtual void stopTimer() override
	{
		KillTimer(m_hWnd, 1000);
		m_interval = -1;
	}

	virtual void setRect(const Rect& rect) override
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

	virtual Rect getRect() const override
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

	virtual Rect getInnerRect() const override
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return Rect(rc.left, rc.top, rc.right, rc.bottom);
	}

	virtual Rect getNormalRect() const override
	{
		WINDOWPLACEMENT wp;
		std::memset(&wp, 0, sizeof(wp));
		wp.length = sizeof(wp);
		GetWindowPlacement(m_hWnd, &wp);
		const RECT rc = wp.rcNormalPosition;
		return Rect(rc.left, rc.top, rc.right, rc.bottom);
	}

	virtual void setFont(const Font& font) override
	{
		LOGFONT lf;

		int32_t dip = font.getSize();
		float inches = dip / 96.0f;
		float logical = inches * getSystemDPI();

		std::memset(&lf, 0, sizeof(lf));
		lf.lfHeight = -int32_t(logical + 0.5f);
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

	virtual Font getFont() const override
	{
		LOGFONT lf;

		BOOL result = GetObject(m_hWnd.getFont(), sizeof(lf), &lf);
		T_ASSERT_M (result, L"Unable to get device font");

		HDC hDC = GetDC(m_hWnd);

		int32_t logical = 0;
		if (lf.lfHeight >= 0)
		{
			TEXTMETRIC tm = { 0 };
			GetTextMetrics(hDC, &tm);
			logical = lf.lfHeight - tm.tmInternalLeading;
		}
		else
			logical = -lf.lfHeight;

		ReleaseDC(m_hWnd, hDC);

		float inches = float(logical) / getSystemDPI();
		float dip = inches * 96.0f;

		return Font(
			tstows(lf.lfFaceName),
			int32_t(dip + 0.5f),
			bool(lf.lfWeight == FW_BOLD),
			bool(lf.lfItalic == TRUE),
			bool(lf.lfUnderline == TRUE)
		);
	}

	virtual const IFontMetric* getFontMetric() const override
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) override
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
		m_hCursor = hCursor;
	}

	virtual Point getMousePosition(bool relative) const override
	{
		POINT pnt;
		GetCursorPos(&pnt);
		if (relative)
			ScreenToClient(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		POINT pnt = { pt.x, pt.y };
		ScreenToClient(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		POINT pnt = { pt.x, pt.y };
		ClientToScreen(m_hWnd, &pnt);
		return Point(pnt.x, pnt.y);
	}

	virtual bool hitTest(const Point& pt) const override
	{
		POINT pnt = { pt.x, pt.y };
		return bool(WindowFromPoint(pnt) == m_hWnd);
	}

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count) override
	{
		HDWP hdwp = BeginDeferWindowPos((int)count);
		if (hdwp)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				hdwp = DeferWindowPos(
					hdwp,
					(HWND)childRects[i].widget->getInternalHandle(),
					NULL,
					childRects[i].rect.left,
					childRects[i].rect.top,
					childRects[i].rect.getWidth(),
					childRects[i].rect.getHeight(),
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
		for (uint32_t i = 0; i < count; ++i)
		{
			SetWindowPos(
				(HWND)childRects[i].widget->getInternalHandle(),
				NULL,
				childRects[i].rect.left,
				childRects[i].rect.top,
				childRects[i].rect.getWidth(),
				childRects[i].rect.getHeight(),
				SWP_NOZORDER | SWP_NOACTIVATE
			);
		}
	}

	virtual Size getMinimumSize() const override
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const override
	{
		return Size(0, 0);
	}

	virtual Size getMaximumSize() const override
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) override
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

	virtual void* getInternalHandle() override
	{
		return static_cast< void* >((HWND)m_hWnd);
	}

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow(m_hWnd);
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override
	{
		T_FATAL_ASSERT(m_canvasImpl != nullptr);
		m_canvasImpl->getAscentAndDescent(m_hWnd, outAscent, outDescent);
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override
	{
		T_FATAL_ASSERT(m_canvasImpl != nullptr);
		return m_canvasImpl->getAdvance(m_hWnd, ch, next);
	}

	virtual int32_t getLineSpacing() const override
	{
		T_FATAL_ASSERT(m_canvasImpl != nullptr);
		return m_canvasImpl->getLineSpacing(m_hWnd);
	}

	virtual Size getExtent(const std::wstring& text) const override
	{
		T_FATAL_ASSERT(m_canvasImpl != nullptr);
		return m_canvasImpl->getExtent(m_hWnd, text);
	}

protected:
	EventSubject* m_owner;
	mutable Window m_hWnd;
	bool m_doubleBuffer;
	CanvasWin32* m_canvasImpl;
	SmartFont m_hFont;
	HCURSOR m_hCursor;
	bool m_ownCursor;
	bool m_tracking;
	int32_t m_interval;

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

		if ((style & WsNoCanvas) == 0)
		{
#if defined(T_USE_DIRECT2D)
#	if defined(T_USE_GDI_PLUS)
			if (style & WsAccelerated)
#	endif
				m_canvasImpl = new CanvasDirect2DWin32();
#endif

#if defined(T_USE_GDI_PLUS)
			if (!m_canvasImpl)
				m_canvasImpl = new CanvasGdiPlusWin32();
#endif

#if defined(T_USE_GDI)
			if (!m_canvasImpl)
				m_canvasImpl = new CanvasGdiWin32();
#endif

			if (!m_canvasImpl)
				return false;
		}

		m_hWnd.registerMessageHandler(WM_CHAR,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventChar));
		m_hWnd.registerMessageHandler(WM_KEYDOWN,       new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventKeyDown));
		m_hWnd.registerMessageHandler(WM_KEYUP,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventKeyUp));
		m_hWnd.registerMessageHandler(WM_MOVE,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMove));
		m_hWnd.registerMessageHandler(WM_SIZE,          new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventSize));
		m_hWnd.registerMessageHandler(WM_LBUTTONDOWN,   new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDown));
		m_hWnd.registerMessageHandler(WM_LBUTTONUP,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonUp));
		m_hWnd.registerMessageHandler(WM_LBUTTONDBLCLK, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDblClk));
		m_hWnd.registerMessageHandler(WM_MBUTTONDOWN,   new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDown));
		m_hWnd.registerMessageHandler(WM_MBUTTONUP,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonUp));
		m_hWnd.registerMessageHandler(WM_MBUTTONDBLCLK, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDblClk));
		m_hWnd.registerMessageHandler(WM_RBUTTONDOWN,   new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDown));
		m_hWnd.registerMessageHandler(WM_RBUTTONUP,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonUp));
		m_hWnd.registerMessageHandler(WM_RBUTTONDBLCLK, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventButtonDblClk));
		m_hWnd.registerMessageHandler(WM_MOUSEMOVE,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMouseMove));
		m_hWnd.registerMessageHandler(WM_MOUSELEAVE,    new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMouseLeave));
		m_hWnd.registerMessageHandler(WM_MOUSEWHEEL,    new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventMouseWheel));
		m_hWnd.registerMessageHandler(WM_SETFOCUS,      new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventFocus));
		m_hWnd.registerMessageHandler(WM_KILLFOCUS,     new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventFocus));
		m_hWnd.registerMessageHandler(WM_PAINT,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventPaint));
		m_hWnd.registerMessageHandler(WM_ERASEBKGND,    new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventEraseBkGnd));
		m_hWnd.registerMessageHandler(WM_TIMER,         new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventTimer));
		m_hWnd.registerMessageHandler(WM_DROPFILES,		new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventDropFiles));

		if (style & WsWantAllInput)
			m_hWnd.registerMessageHandler(WM_GETDLGCODE, new MethodMessageHandler< WidgetWin32Impl >(this, &WidgetWin32Impl::eventGetDlgCode));

		setCursor(CrArrow);
		return true;
	}

	LRESULT eventChar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		// Seems weird but Windows send Nth character in alphabet when holding down CTRL.
		if (wParam < L'A' && GetAsyncKeyState(VK_CONTROL))
			wParam = L'A' - 1 + wParam;

		KeyEvent k(m_owner, translateToVirtualKey(int(wParam)), int(wParam), wchar_t(wParam));
		m_owner->raiseEvent(&k);
		if (!k.consumed())
			outPass = true;

		return TRUE;
	}

	LRESULT eventKeyDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		KeyDownEvent k(m_owner, translateToVirtualKey(int(wParam)), int(wParam), 0);
		m_owner->raiseEvent(&k);
		if (!k.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventKeyUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		KeyUpEvent k(m_owner, translateToVirtualKey(int(wParam)), int(wParam), 0);
		m_owner->raiseEvent(&k);
		if (!k.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		MoveEvent m(m_owner, Point(LOWORD(lParam), HIWORD(lParam)));
		m_owner->raiseEvent(&m);
		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		SizeEvent s(m_owner, Size(LOWORD(lParam), HIWORD(lParam)));
		m_owner->raiseEvent(&s);
		if (!s.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		int32_t button = MbtNone;
		switch (message)
		{
		case WM_LBUTTONDOWN:
			button = MbtLeft;
			break;
		case WM_MBUTTONDOWN:
			button = MbtMiddle;
			break;
		case WM_RBUTTONDOWN:
			button = MbtRight;
			break;
		}

		MouseButtonDownEvent m(
			m_owner,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(&m);

		if (!m.consumed())
			outPass = true;

		return TRUE;
	}

	LRESULT eventButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		int32_t button = MbtNone;
		switch (message)
		{
		case WM_LBUTTONUP:
			button = MbtLeft;
			break;
		case WM_MBUTTONUP:
			button = MbtMiddle;
			break;
		case WM_RBUTTONUP:
			button = MbtRight;
			break;
		}

		MouseButtonUpEvent m(
			m_owner,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(&m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventButtonDblClk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		int32_t button = MbtNone;
		switch (message)
		{
		case WM_LBUTTONDBLCLK:
			button = MbtLeft;
			break;
		case WM_MBUTTONDBLCLK:
			button = MbtMiddle;
			break;
		case WM_RBUTTONDBLCLK:
			button = MbtRight;
			break;
		}

		MouseDoubleClickEvent m(
			m_owner,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(&m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (!m_tracking)
		{
			TRACKMOUSEEVENT tme = { 0 };
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			tme.dwHoverTime = 0;
			if (TrackMouseEvent(&tme))
			{
				MouseTrackEvent m(m_owner, true);
				m_owner->raiseEvent(&m);
				m_tracking = true;
			}
		}

		int32_t button = MbtNone;
		if (wParam & MK_LBUTTON)
			button |= MbtLeft;
		if (wParam & MK_MBUTTON)
			button |= MbtMiddle;
		if (wParam & MK_RBUTTON)
			button |= MbtRight;

		MouseMoveEvent m(
			m_owner,
			button,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(&m);

		if (!m.consumed())
			outPass = true;

		if (m_ownCursor)
			SetCursor(m_hCursor);

		return TRUE;
	}

	LRESULT eventMouseLeave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (m_tracking)
		{
			MouseTrackEvent m(m_owner, false);
			m_owner->raiseEvent(&m);
			m_tracking = false;
		}
		return 0;
	}

	LRESULT eventMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		MouseWheelEvent m(
			m_owner,
			GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA,
			Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
		);
		m_owner->raiseEvent(&m);

		if (!m.consumed())
			outPass = true;
		return TRUE;
	}

	LRESULT eventFocus(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		FocusEvent focusEvent(m_owner, bool(message == WM_SETFOCUS));
		m_owner->raiseEvent(&focusEvent);

		outPass = true;
		return TRUE;
	}

	LRESULT eventPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (m_owner->hasEventHandler< PaintEvent >())
		{
			RECT rcUpdate = { 0 };
			GetUpdateRect(m_hWnd, &rcUpdate, FALSE);

			if (m_canvasImpl != nullptr && m_canvasImpl->beginPaint(m_hWnd, m_doubleBuffer, NULL))
			{
				Canvas canvas(m_canvasImpl);
				PaintEvent p(
					m_owner,
					canvas,
					Rect(rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom)
				);
				m_owner->raiseEvent(&p);
				m_canvasImpl->endPaint(m_hWnd);
				outPass = !p.consumed();
			}
			else if (m_canvasImpl == nullptr)
			{
				Canvas canvas(nullptr);
				PaintEvent p(
					m_owner,
					canvas,
					Rect(rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom)
				);
				m_owner->raiseEvent(&p);
				outPass = !p.consumed();
			}

			ValidateRect(m_hWnd, NULL);
		}
		else
			outPass = true;

		return 0;
	}

	LRESULT eventEraseBkGnd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		if (m_owner->hasEventHandler< PaintEvent >())
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
			return 0;

		// Disable all timers until event is processed.
		KillTimer(m_hWnd, 1000);

		TimerEvent c(m_owner);
		m_owner->raiseEvent(&c);
		if (!c.consumed())
			outPass = true;

		// Restart all timers again.
		if (m_interval > 0)
			SetTimer(m_hWnd, 1000, m_interval, NULL);

		return 0;
	}

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

		FileDropEvent e(m_owner, files);
		m_owner->raiseEvent(&e);
		if (!e.consumed())
		{
			outPass = true;
			return TRUE;
		}

		DragFinish(hDrop);
		return FALSE;
	}

	LRESULT eventGetDlgCode(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
	{
		return DLGC_WANTMESSAGE;
	}
};

	}
}

