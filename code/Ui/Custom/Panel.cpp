#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Panel", Panel, Container)

Panel::Panel()
:	m_focusHandler(0)
{
}

bool Panel::create(Widget* parent, const std::wstring& text, Layout* layout)
{
	if (!Container::create(parent, WsNone, layout))
		return false;

	setText(text);

	addPaintEventHandler(createMethodHandler(this, &Panel::eventPaint));

	m_focusHandler = createMethodHandler(this, &Panel::eventFocus);
	Application::getInstance()->addEventHandler(EiFocus, m_focusHandler);

	return true;
}

void Panel::destroy()
{
	if (m_focusHandler)
	{
		Application::getInstance()->removeEventHandler(EiFocus, m_focusHandler);
		m_focusHandler = 0;
	}
	Widget::destroy();
}

Size Panel::getMinimumSize() const
{
	Size titleSize = getTextExtent(getText());
	Size sz = Container::getMinimumSize();
	sz.cx += 2;
	sz.cy += 1 + titleSize.cy;
	return sz;
}

Size Panel::getPreferedSize() const
{
	Size titleSize = getTextExtent(getText());
	Size sz = Container::getPreferedSize();
	sz.cx += 2;
	sz.cy += 3 + titleSize.cy;
	return sz;
}

Rect Panel::getInnerRect() const
{
	Size titleSize = getTextExtent(getText());
	Rect rc = Container::getInnerRect();
	rc.left += 1;
	rc.top += titleSize.cy + 3;
	rc.right -= 1;
	rc.bottom -= 1;
	return rc;
}

void Panel::eventPaint(Event* event)
{
	PaintEvent* p = static_cast< PaintEvent* >(event);
	Canvas& canvas = p->getCanvas();

	Rect rcInner = Widget::getInnerRect();
	canvas.fillRect(rcInner);

	bool focus = containFocus();

	std::wstring text = getText();
	Size extent = canvas.getTextExtent(text);

	Rect rcTitle(rcInner.left, rcInner.top, rcInner.right, rcInner.top + extent.cy + 4);

	canvas.setBackground(getSystemColor(focus ? ScActiveCaption : ScInactiveCaption));
	canvas.fillRect(rcTitle);

	canvas.setForeground(getSystemColor(focus ? ScActiveCaptionText : ScInactiveCaptionText));
	canvas.drawText(
		rcTitle.inflate(-4, 0),
		text,
		AnLeft,
		AnCenter
	);

	Point pntBorder[5] =
	{
		Point(rcInner.left, rcInner.top),
		Point(rcInner.right - 1, rcInner.top),
		Point(rcInner.right - 1, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.top)
	};
	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLines(pntBorder, 5);

	event->consume();
}

void Panel::eventFocus(Event* event)
{
	update();
}

		}
	}
}
