#include "Ui/Auto/AutoWidget.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutoWidgetCell", AutoWidgetCell, ui::EventSubject)

void AutoWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
	m_widget = widget;
}

AutoWidgetCell* AutoWidgetCell::hitTest(const Point& position)
{
	return this;
}

bool AutoWidgetCell::beginCapture()
{
	return true;
}

void AutoWidgetCell::endCapture()
{
}

void AutoWidgetCell::interval()
{
}

void AutoWidgetCell::mouseEnter()
{
}

void AutoWidgetCell::mouseLeave()
{
}

void AutoWidgetCell::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseMove(MouseMoveEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseMoveFocus(MouseMoveEvent* event, const Point& position)
{
}

void AutoWidgetCell::paint(Canvas& canvas, const Rect& rect)
{
}

void AutoWidgetCell::requestWidgetUpdate()
{
	if (m_widget)
		m_widget->requestUpdate();
}

void AutoWidgetCell::raiseWidgetEvent(Event* event)
{
	if (m_widget)
		m_widget->raiseEvent(event);
}

	}
}
