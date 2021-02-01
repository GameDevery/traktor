#pragma once

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/X11/Context.h"
#include "Ui/X11/CanvasX11.h"
#include "Ui/X11/Timers.h"
#include "Ui/X11/TypesX11.h"
#include "Ui/X11/UtilitiesX11.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

template < typename ControlType >
class WidgetX11Impl
:	public ControlType
,	public IFontMetric
{
public:
	WidgetX11Impl(Context* context, EventSubject* owner)
	:	m_context(context)
	,	m_owner(owner)
	,	m_xic(0)
	,	m_surface(nullptr)
	,	m_cairo(nullptr)
	,	m_timer(-1)
	,	m_lastMousePress(0)
	,	m_lastMouseButton(0)
	,	m_pendingExposure(false)
	{
	}

	virtual ~WidgetX11Impl()
	{
		T_FATAL_ASSERT(m_timer < 0);
		T_FATAL_ASSERT(m_surface == nullptr);
		T_FATAL_ASSERT(m_cairo == nullptr);
		T_FATAL_ASSERT(m_data.grabbed == false);
	}

	virtual void destroy() override
	{
		stopTimer();
		releaseCapture();

		if (hasFocus())
			m_context->setFocus(nullptr);

		if (m_context != nullptr)
		{
			m_context->defer([&]() {
				if (m_cairo != nullptr)
				{
					cairo_destroy(m_cairo);
					m_cairo = nullptr;
				}

				if (m_surface != nullptr)
				{
					cairo_surface_destroy(m_surface);
					m_surface = nullptr;
				}

				if (m_xic != 0)
				{
					XDestroyIC(m_xic);
					m_xic = 0;
				}

				XDestroyWindow(m_context->getDisplay(), m_data.window);

				m_context->unbind(&m_data);
				m_context = nullptr;

				m_data.window = None;
				m_data.parent = nullptr;

				delete this;
			});
		}

		m_owner = nullptr;
	}

	virtual void setParent(IWidget* parent) override
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		XReparentWindow(m_context->getDisplay(), m_data.window, parentData->window, 0, 0);
		m_data.parent = parentData;
	}

	virtual void setText(const std::wstring& text) override
	{
		m_text = text;
	}

	virtual std::wstring getText() const override
	{
		return m_text;
	}

	virtual void setForeground() override
	{
	}

	virtual bool isForeground() const override
	{
		return false;
	}

	virtual void setVisible(bool visible) override
	{
		if (visible != m_data.visible)
		{
			m_data.visible = visible;
			if (visible)
			{
				int32_t width = std::max< int32_t >(m_rect.getWidth(), 1);
				int32_t height = std::max< int32_t >(m_rect.getHeight(), 1);

				// Resize window.
				XMapWindow(m_context->getDisplay(), m_data.window);
				XMoveResizeWindow(m_context->getDisplay(), m_data.window, m_rect.left, m_rect.top, width, height);

				// Resize surface.
				cairo_xlib_surface_set_size(m_surface, width, height);
			}
			else
			{
				XUnmapWindow(m_context->getDisplay(), m_data.window);
			}

			ShowEvent showEvent(m_owner, visible);
			m_owner->raiseEvent(&showEvent);
		}
	}

	virtual bool isVisible() const override
	{
		return m_data.visible;
	}

	virtual void setEnable(bool enable) override
	{
		m_data.enable = enable;
	}

	virtual bool isEnable() const override
	{
		return m_data.enable;
	}

	virtual bool hasFocus() const override
	{
		return m_data.focus;
	}

	virtual void setFocus() override
	{
		m_context->setFocus(&m_data);
	}

	virtual bool hasCapture() const override
	{
		return m_data.grabbed;
	}

	virtual void setCapture() override
	{
		if (!m_data.grabbed)
			m_context->grab(&m_data);
	}

	virtual void releaseCapture() override
	{
		if (m_data.grabbed)
			m_context->ungrab(&m_data);
	}

	virtual void startTimer(int interval) override
	{
		stopTimer();
		m_timer = Timers::getInstance().bind(interval, [=](int32_t){
			if (!isVisible())
				return;
			TimerEvent timerEvent(m_owner, 0);
			m_owner->raiseEvent(&timerEvent);
		});
	}

	virtual void stopTimer() override
	{
		if (m_timer >= 0)
		{
			Timers::getInstance().unbind(m_timer);
			m_timer = -1;
		}
	}

	virtual void setRect(const Rect& rect) override
	{
		int32_t oldWidth = std::max< int32_t >(m_rect.getWidth(), 1);
		int32_t oldHeight = std::max< int32_t >(m_rect.getHeight(), 1);

		int32_t newWidth = std::max< int32_t >(rect.getWidth(), 1);
		int32_t newHeight = std::max< int32_t >(rect.getHeight(), 1);

		m_rect = rect;

		if (m_data.visible)
		{
			if (newWidth != oldWidth || newHeight != oldHeight)
			{
				XMoveResizeWindow(m_context->getDisplay(), m_data.window, rect.left, rect.top, newWidth, newHeight);
			 	cairo_xlib_surface_set_size(m_surface, newWidth, newHeight);
			}
			else
				XMoveWindow(m_context->getDisplay(), m_data.window, rect.left, rect.top);
		}

		if (newWidth != oldWidth || newHeight != oldHeight)
		{
			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);
		}
	}

	virtual Rect getRect() const override
	{
		return m_rect;
	}

	virtual Rect getInnerRect() const override
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual Rect getNormalRect() const override
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual void setFont(const Font& font) override
	{
		m_font = font;

		cairo_select_font_face(
			m_cairo,
			wstombs(m_font.getFace()).c_str(),
			CAIRO_FONT_SLANT_NORMAL,
			m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
		);

		cairo_set_font_size(
			m_cairo,
			dpi96(m_font.getSize())
		);
	}

	virtual Font getFont() const override
	{
		return m_font;
	}

	virtual const IFontMetric* getFontMetric() const override
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) override
	{
	}

	virtual Point getMousePosition(bool relative) const override
	{
		Window root, child;
		int rootX, rootY;
		int winX, winY;
		unsigned int mask;

		XQueryPointer(
			m_context->getDisplay(),
			relative ? m_data.window : DefaultRootWindow(m_context->getDisplay()),
			&root,
			&child,
			&rootX, &rootY,
			&winX, &winY,
			&mask
		);

		return Point(winX, winY);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_context->getDisplay(), DefaultRootWindow(m_context->getDisplay()), m_data.window, pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_context->getDisplay(), m_data.window, DefaultRootWindow(m_context->getDisplay()), pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual bool hitTest(const Point& pt) const override
	{
		Point cpt = screenToClient(pt);
		return getInnerRect().inside(cpt);
	}

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count) override
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			if (childRects[i].widget)
				childRects[i].widget->setRect(childRects[i].rect);
		}
	}

	virtual Size getMinimumSize() const override
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const override
	{
		return Size(128, 64);
	}

	virtual Size getMaximumSize() const override
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) override
	{
		if (!m_data.visible)
			return;

		if (!immediate)
		{
			if (m_pendingExposure)
				return;

			m_pendingExposure = true;

			XEvent xe = { 0 };
        	xe.type = Expose;
        	xe.xexpose.window = m_data.window;
        	XSendEvent(m_context->getDisplay(), m_data.window, False, ExposureMask, &xe);
		}
		else
		{
			draw(rc);
		}
	}

	virtual void* getInternalHandle() override
	{
		return (void*)&m_data;
	}

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow(m_context->getDisplay(), m_data.window);
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_font_extents_t x;
		cairo_font_extents(m_cairo, &x);

		outAscent = (int32_t)x.ascent;
		outDescent = (int32_t)x.descent;
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		uint8_t uc[IEncoding::MaxEncodingSize + 1] = { 0 };
		int32_t nuc = Utf8Encoding().translate(&ch, 1, uc);
		if (nuc <= 0)
			return 0;

		cairo_text_extents_t tx;
		cairo_text_extents(m_cairo, (const char*)uc, &tx);

		return (int32_t)tx.x_advance;
	}

	virtual int32_t getLineSpacing() const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_font_extents_t x;
		cairo_font_extents(m_cairo, &x);

		return (int32_t)x.height;
	}

	virtual Size getExtent(const std::wstring& text) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_font_extents_t fx;
		cairo_text_extents_t tx;
		cairo_font_extents(m_cairo, &fx);
		cairo_text_extents(m_cairo, wstombs(text).c_str(), &tx);

		return Size(tx.width, fx.height);
	}

protected:
	enum
	{
		_NET_WM_STATE_REMOVE = 0,
		_NET_WM_STATE_ADD = 1,
		_NET_WM_STATE_TOGGLE = 2
	};

	Ref< Context > m_context;
	EventSubject* m_owner;

	WidgetData m_data;
	XIC m_xic;

	Rect m_rect;
	Font m_font;

	cairo_surface_t* m_surface;
	cairo_t* m_cairo;

	std::wstring m_text;
	int32_t m_timer;

	int32_t m_lastMousePress;
	int32_t m_lastMouseButton;
	bool m_pendingExposure;

	bool create(IWidget* parent, int32_t style, Window window, const Rect& rect, bool visible, bool topLevel)
	{
		if (window == None)
			return false;

		m_data.window = window;
		m_data.parent = (parent != nullptr ? static_cast< WidgetData* >(parent->getInternalHandle()) : nullptr);
		m_data.topLevel = topLevel;
		m_data.visible = visible;

		m_rect = rect;

		if (topLevel)
		{
			XSelectInput(
				m_context->getDisplay(),
				m_data.window,
				ButtonPressMask |
				ButtonReleaseMask |
				StructureNotifyMask |
				KeyPressMask |
				KeyReleaseMask |
				ExposureMask |
				FocusChangeMask |
				PointerMotionMask |
				EnterWindowMask |
				LeaveWindowMask
			);
		}
		else
		{
			XSelectInput(
				m_context->getDisplay(),
				m_data.window,
				ButtonPressMask |
				ButtonReleaseMask |
				KeyPressMask |
				KeyReleaseMask |
				ExposureMask |
				FocusChangeMask |
				PointerMotionMask |
				EnterWindowMask |
				LeaveWindowMask
			);
		}

		if (visible)
	    	XMapWindow(m_context->getDisplay(), m_data.window);

		XFlush(m_context->getDisplay());

		// Create input context.
		if ((m_xic = XCreateIC(
			m_context->getXIM(),
			XNInputStyle,   XIMPreeditNothing | XIMStatusNothing,
			XNClientWindow, m_data.window,
			XNFocusWindow,  m_data.window,
			nullptr
		)) == 0)
		{
			return false;
		}

		m_surface = cairo_xlib_surface_create(
			m_context->getDisplay(),
			m_data.window,
			DefaultVisual(m_context->getDisplay(), m_context->getScreen()),
			m_rect.getWidth(),
			m_rect.getHeight()
		);

		m_cairo = cairo_create(m_surface);
		setFont(Font(L"Ubuntu Regular", 11));

		// Focus in.
		m_context->bind(&m_data, FocusIn, [&](XEvent& xe) {
			XSetICFocus(m_xic);
			FocusEvent focusEvent(m_owner, true);
			m_owner->raiseEvent(&focusEvent);
		});

		// Focus out.
		m_context->bind(&m_data, FocusOut, [&](XEvent& xe) {
			XUnsetICFocus(m_xic);
			FocusEvent focusEvent(m_owner, false);
			m_owner->raiseEvent(&focusEvent);
		});

		// Key press.
		m_context->bind(&m_data, KeyPress, [&](XEvent& xe) {
			T_FATAL_ASSERT (m_data.enable);

			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), xe.xkey.keycode, 1, &nkeysyms);
			if (ks != nullptr)
			{
				VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
				if (vk != VkNull)
				{
					KeyDownEvent keyDownEvent(m_owner, vk, xe.xkey.keycode, 0);
					m_owner->raiseEvent(&keyDownEvent);
				}

				// Ensure owner is still valid; widget might have been destroyed in key down event.
				if (m_owner)
				{
					uint8_t str[8] = { 0 };

					Status status = 0;
					const int n = Xutf8LookupString(m_xic, &xe.xkey, (char*)str, 8, ks, &status);
					if (n > 0)
					{
						wchar_t wch = 0;
						if (Utf8Encoding().translate(str, n, wch) > 0)
						{
							KeyEvent keyEvent(m_owner, vk, xe.xkey.keycode, wch);
							m_owner->raiseEvent(&keyEvent);
						}
					}
				}

				XFree(ks);
			}
		});

		// Key release.
		m_context->bind(&m_data, KeyRelease, [&](XEvent& xe) {
			T_FATAL_ASSERT (m_data.enable);

			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), xe.xkey.keycode, 1, &nkeysyms);
			if (ks != nullptr)
			{
				VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
				if (vk != VkNull)
				{
					KeyUpEvent keyUpEvent(m_owner, vk, xe.xkey.keycode, 0);
					m_owner->raiseEvent(&keyUpEvent);
				}

				XFree(ks);
			}
		});

		// Motion
		m_context->bind(&m_data, MotionNotify, [&](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			int32_t button = 0;
			if ((xe.xmotion.state & Button1Mask) != 0)
				button = MbtLeft;
			if ((xe.xmotion.state & Button2Mask) != 0)
				button = MbtMiddle;
			if ((xe.xmotion.state & Button3Mask) != 0)
				button = MbtRight;

			MouseMoveEvent mouseMoveEvent(
				m_owner,
				button,
				Point(xe.xmotion.x, xe.xmotion.y)
			);
			m_owner->raiseEvent(&mouseMoveEvent);
		});

		// Enter
		m_context->bind(&m_data, EnterNotify, [&](XEvent& xe){
			MouseTrackEvent mouseTrackEvent(m_owner, true);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		// Leave
		m_context->bind(&m_data, LeaveNotify, [&](XEvent& xe){
			MouseTrackEvent mouseTrackEvent(m_owner, false);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		// Button press.
		m_context->bind(&m_data, ButtonPress, [&](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			if (xe.xbutton.button == 4 || xe.xbutton.button == 5)
			{
				MouseWheelEvent mouseWheelEvent(
					m_owner,
					xe.xbutton.button == 4 ? 1 : -1,
					clientToScreen(Point(xe.xbutton.x, xe.xbutton.y))
				);
				m_owner->raiseEvent(&mouseWheelEvent);
			}
			else
			{
				int32_t button = 0;
				switch (xe.xbutton.button)
				{
				case Button1:
					button = MbtLeft;
					break;

				case Button2:
					button = MbtMiddle;
					break;

				case Button3:
					button = MbtRight;
					break;

				default:
					return;
				}

				setFocus();

				MouseButtonDownEvent mouseButtonDownEvent(
					m_owner,
					button,
					Point(xe.xbutton.x, xe.xbutton.y)
				);
				m_owner->raiseEvent(&mouseButtonDownEvent);

				int32_t dbt = xe.xbutton.time - m_lastMousePress;
				if (dbt <= 200 && m_lastMouseButton == button)
				{
					MouseDoubleClickEvent mouseDoubleClickEvent(
						m_owner,
						button,
						Point(xe.xbutton.x, xe.xbutton.y)
					);
					m_owner->raiseEvent(&mouseDoubleClickEvent);
				}

				m_lastMousePress = xe.xbutton.time;
				m_lastMouseButton = button;
			}
		});

		// Button release.
		m_context->bind(&m_data, ButtonRelease, [&](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			int32_t button = 0;
			switch (xe.xbutton.button)
			{
			case Button1:
				button = MbtLeft;
				break;

			case Button2:
				button = MbtMiddle;
				break;

			case Button3:
				button = MbtRight;
				break;

			default:
				return;
			}

			MouseButtonUpEvent mouseButtonUpEvent(
				m_owner,
				button,
				Point(xe.xbutton.x, xe.xbutton.y)
			);
			m_owner->raiseEvent(&mouseButtonUpEvent);
		});

		// Configure
		if (topLevel)
		{
			m_context->bind(&m_data, ConfigureNotify, [&](XEvent& xe){
				Rect rect(
					Point(xe.xconfigure.x, xe.xconfigure.y),
					Size(xe.xconfigure.width, xe.xconfigure.height)
				);

				int32_t oldWidth = std::max< int32_t >(m_rect.getWidth(), 1);
				int32_t oldHeight = std::max< int32_t >(m_rect.getHeight(), 1);

				int32_t newWidth = std::max< int32_t >(rect.getWidth(), 1);
				int32_t newHeight = std::max< int32_t >(rect.getHeight(), 1);

				if (oldWidth != newWidth || oldHeight != newHeight)
				{
					m_rect = rect;

					if (m_data.visible)
						cairo_xlib_surface_set_size(m_surface, newWidth, newHeight);

					SizeEvent sizeEvent(m_owner, m_rect.getSize());
					m_owner->raiseEvent(&sizeEvent);
				}
			});
		}

		// Expose
		m_context->bind(&m_data, Expose, [&](XEvent& xe){
			if (xe.xexpose.count != 0)
				return;
			draw(nullptr);
		});

		return true;
	}

	void draw(const Rect* rc)
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		m_pendingExposure = false;

		XWindowAttributes xa;
		XGetWindowAttributes(m_context->getDisplay(), m_data.window, &xa);
		if (xa.map_state != IsViewable)
			return;

		Size sz = m_rect.getSize();
		if (sz.cx <= 0 || sz.cy <= 0)
			return;

		cairo_push_group(m_cairo);

		CanvasX11 canvasImpl(m_cairo);
		Canvas canvas(&canvasImpl);
		PaintEvent paintEvent(
			m_owner,
			canvas,
			rc != nullptr ? *rc : Rect(Point(0, 0), sz)
		);
		m_owner->raiseEvent(&paintEvent);

#if 0
		if (m_data.grabbed)
		{
			canvas.setBackground(Color4ub(255, 0, 0, 128));
			canvas.fillRect(Rect(Point(0, 0), sz));
		}
		else if (m_data.focus)
		{
			canvas.setBackground(Color4ub(0, 0, 255, 128));
			canvas.fillRect(Rect(Point(0, 0), sz));
		}
#endif

		cairo_pop_group_to_source(m_cairo);
		cairo_paint(m_cairo);

		cairo_surface_flush(m_surface);
	}

	void setWmProperty(const char* const property, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
		Atom atomWmProperty = XInternAtom(m_context->getDisplay(), property, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_data.window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty;
		evt.xclient.data.l[2] = 0;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_context->getDisplay(),
			RootWindow(m_context->getDisplay(), m_context->getScreen()),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}

	void setWmProperty(const char* const property1, const char* const property2, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
		Atom atomWmProperty1 = XInternAtom(m_context->getDisplay(), property1, False);
		Atom atomWmProperty2 = XInternAtom(m_context->getDisplay(), property2, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_data.window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty1;
		evt.xclient.data.l[2] = atomWmProperty2;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_context->getDisplay(),
			RootWindow(m_context->getDisplay(), m_context->getScreen()),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}
};

	}
}
