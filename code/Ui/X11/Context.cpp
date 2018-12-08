#include <algorithm>
#include "Core/Assert.h"
#include "Ui/X11/Context.h"

namespace traktor
{
    namespace ui
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Context", Context, Object)

Context::Context(Display* display, int screen, XIM xim)
:	m_display(display)
,	m_screen(screen)
,	m_xim(xim)
,	m_grabbed(nullptr)
,	m_focused(nullptr)
{
}

void Context::bind(WidgetData* widget, int32_t eventType, const std::function< void(XEvent& xe) >& fn)
{
	auto& b = m_bindings[widget->window];
	b.widget = widget;
	b.fns[eventType] = fn;
}

void Context::unbind(WidgetData* widget)
{
	auto it = std::find(m_modal.begin(), m_modal.end(), widget);
	if (it != m_modal.end())
		m_modal.erase(it);

	m_bindings.erase(widget->window);
	
	if (m_grabbed == widget)
		m_grabbed = nullptr;

	if (m_focused == widget)
		m_focused = nullptr;
}

void Context::defer(const std::function< void() >& fn)
{
	m_deferred.push_back(fn);
}

void Context::pushModal(WidgetData* widget)
{
	m_modal.push_back(widget);
}

void Context::popModal()
{
	T_FATAL_ASSERT(!m_modal.empty());
	m_modal.pop_back();
}

void Context::grab(WidgetData* widget)
{
	T_FATAL_ASSERT(!widget->grabbed);

#if !defined(_DEBUG)
	widget->grabbed = bool(XGrabPointer(
		m_display,
		widget->window,
		False,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync,
		GrabModeAsync,
		None,
		None,
		CurrentTime
	) == GrabSuccess);
#else
	widget->grabbed = true;
#endif

	if (widget->grabbed)
	{
		if (m_grabbed)
		{
			m_grabbed->grabbed = false;
			m_grabbed = nullptr;
		}
		m_grabbed = widget;
	}
}

void Context::ungrab(WidgetData* widget)
{
	T_FATAL_ASSERT(widget->grabbed);
	T_FATAL_ASSERT(widget == m_grabbed);

#if !defined(_DEBUG)
	XUngrabPointer(m_display, CurrentTime);
#endif

	widget->grabbed = false;
	m_grabbed = nullptr;
}

void Context::setFocus(WidgetData* widget)
{
	if (m_focused != nullptr)
	{
		if (m_focused == widget)
			return;

		m_focused->focus = false;

		XEvent xevent;
		xevent.type = FocusOut;
		dispatch(m_focused->window, FocusOut, true, xevent);

		m_focused = nullptr;
	}

	if (widget != nullptr)
	{
		m_focused = widget;
		m_focused->focus = true;

		XSetInputFocus(m_display, m_focused->window, RevertToNone, CurrentTime);

		XEvent xevent;
		xevent.type = FocusIn;
		dispatch(m_focused->window, FocusIn, true, xevent);
	}
}

void Context::dispatch(XEvent& xe)
{
	switch (xe.type)
    {
    case FocusIn:
		{
			Window focusWindow; int revertTo;
			XGetInputFocus(m_display, &focusWindow, &revertTo);
			
			if (m_focused != nullptr)
			{
				if (m_focused->window == focusWindow)
					break;

				m_focused->focus = false;

				XEvent xevent;
				xevent.type = FocusOut;
				dispatch(m_focused->window, FocusOut, true, xevent);

				m_focused = nullptr;
			}

			auto b = m_bindings.find(focusWindow);
			if (b != m_bindings.end())
			{
				m_focused = b->second.widget;
				m_focused->focus = true;

				XEvent xevent;
				xevent.type = FocusIn;
				dispatch(m_focused->window, FocusIn, true, xevent);				
			}
		}
        break;

    case KeyPress:
        dispatch(xe.xkey.window, KeyPress, false, xe);
        break;

    case KeyRelease:
        dispatch(xe.xkey.window, KeyRelease, false, xe);
        break;

    case MotionNotify:
        dispatch(xe.xmotion.window, MotionNotify, false, xe);
        break;

	case EnterNotify:
		dispatch(xe.xcrossing.window, EnterNotify, false, xe);
		break;

	case LeaveNotify:
		dispatch(xe.xcrossing.window, LeaveNotify, false, xe);
		break;

    case ButtonPress:
        dispatch(xe.xbutton.window, ButtonPress, false, xe);
        break;

    case ButtonRelease:
        dispatch(xe.xbutton.window, ButtonRelease, false, xe);
        break;

    case ConfigureRequest:
        dispatch(xe.xconfigurerequest.window, ConfigureRequest, true, xe);
        break;

    case ConfigureNotify:
        dispatch(xe.xconfigure.window, ConfigureNotify, true, xe);
        break;
        
    case Expose:
        dispatch(xe.xexpose.window, Expose, true, xe);
        break;

    case ClientMessage:
        dispatch(xe.xclient.window, ClientMessage, true, xe);
        break;

    default:
        break;
    }

	for (auto fn : m_deferred)
		fn();

	m_deferred.clear();
}

void Context::dispatch(Window window, int32_t eventType, bool always, XEvent& xe)
{
    auto b = m_bindings.find(window);
    if (b == m_bindings.end())
        return;

	T_FATAL_ASSERT(b->second.widget != nullptr);

	if (!always && b->second.widget != m_grabbed)
	{
		// If widget or parents is disabled then ignore event.
		for (const WidgetData* w = b->second.widget; w != nullptr; w = w->parent)
		{
			if (!w->enable)
				return;			
		}

		// If exclusive filtering is enabled then ensure widget is part of exclusive.
		if (!m_modal.empty())
		{
			bool p = false;
			for (const WidgetData* w = b->second.widget; w != nullptr; w = w->parent)
			{
				if (w == m_modal.back())
				{
					p = true;
					break;
				}
			}
			if (!p)
				return;
		}
	}

	auto d = b->second.fns.find(eventType);
	if (d == b->second.fns.end())
		return;

	std::function< void(XEvent& xe) > fn = d->second;
	fn(xe);
}

    }
}