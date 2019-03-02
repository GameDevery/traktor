#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Static", Static, Widget)

bool Static::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent))
		return false;

	setText(text);
	addEventHandler< PaintEvent >(this, &Static::eventPaint);
	return true;
}

Size Static::getPreferedSize() const
{
	return getFontMetric().getExtent(getText()) + Size(dpi96(1), dpi96(1));
}

Size Static::getMaximumSize() const
{
	return getPreferedSize();
}

void Static::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Rect rcInner = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(rcInner, getText(), AnLeft, AnCenter);

	event->consume();
}

	}
}
