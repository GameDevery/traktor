#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Splitter.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

Widget* findVisibleSibling(Widget* widget)
{
	while (widget != nullptr && !widget->isVisible(false))
		widget = widget->getNextSibling();
	return widget;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Splitter", Splitter, Widget)

const int c_splitterSize = 2;

Splitter::Splitter()
:	m_vertical(true)
,	m_position(0)
,	m_negative(false)
,	m_relative(false)
,	m_border(0)
,	m_drag(false)
{
}

bool Splitter::create(Widget* parent, bool vertical, int position, bool relative, int border)
{
	if (!Widget::create(parent))
		return false;

	m_vertical = vertical;
	m_position = (position < 0) ? -position : position;
	m_negative = (position < 0) ? true : false;
	m_relative = relative;
	m_border = border;
	m_drag = false;

	addEventHandler< MouseMoveEvent >(this, &Splitter::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Splitter::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Splitter::eventButtonUp);
	addEventHandler< SizeEvent >(this, &Splitter::eventSize);
	addEventHandler< PaintEvent >(this, &Splitter::eventPaint);

	return true;
}

Size Splitter::getMinimumSize() const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx += left->getMinimumSize().cx;
			size.cy = std::max< int >(size.cy, left->getMinimumSize().cy);
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx += right->getMinimumSize().cx;
			size.cy = std::max< int >(size.cy, right->getMinimumSize().cy);
		}
	}
	else
	{
		size.cy = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx = std::max< int >(size.cx, left->getMinimumSize().cx);
			size.cy += left->getMinimumSize().cy;
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx = std::max< int >(size.cx, right->getMinimumSize().cx);
			size.cy += right->getMinimumSize().cy;
		}
	}
	return size;
}

Size Splitter::getPreferedSize() const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx += left->getPreferedSize().cx;
			size.cy = std::max< int >(size.cy, left->getPreferedSize().cy);
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx += right->getPreferedSize().cx;
			size.cy = std::max< int >(size.cy, right->getPreferedSize().cy);
		}
	}
	else
	{
		size.cy = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx = std::max< int >(size.cx, left->getPreferedSize().cx);
			size.cy += left->getPreferedSize().cy;
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx = std::max< int >(size.cx, right->getPreferedSize().cx);
			size.cy += right->getPreferedSize().cy;
		}
	}
	return size;
}

Size Splitter::getMaximumSize() const
{
	Size size(0, 0);
	if (m_vertical == true)
	{
		size.cx = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx += left->getMaximumSize().cx;
			size.cy = std::max< int >(size.cy, left->getMaximumSize().cy);
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx += right->getMaximumSize().cx;
			size.cy = std::max< int >(size.cy, right->getMaximumSize().cy);
		}
	}
	else
	{
		size.cy = ui::dpi96(c_splitterSize);

		Widget* left = getLeftWidget();
		if (left != nullptr)
		{
			size.cx = std::max< int >(size.cx, left->getMaximumSize().cx);
			size.cy += left->getMaximumSize().cy;
		}

		Widget* right = getRightWidget();
		if (right != nullptr)
		{
			size.cx = std::max< int >(size.cx, right->getMaximumSize().cx);
			size.cy += right->getMaximumSize().cy;
		}
	}
	return size;
}

void Splitter::update(const Rect* rc, bool immediate)
{
	Widget* left = getLeftWidget();
	Widget* right = getRightWidget();

	Rect inner = getInnerRect();
	if (left != nullptr && right != nullptr)
	{
		int position = getAbsolutePosition();

		Rect rcLeft(0, 0, 0, 0);
		if (m_vertical == true)
			rcLeft.setSize(Size(position - ui::dpi96(c_splitterSize) / 2, inner.getHeight()));
		else
			rcLeft.setSize(Size(inner.getWidth(), position - ui::dpi96(c_splitterSize) / 2));
		left->setRect(rcLeft);

		Rect rcRight(0, 0, 0, 0);
		if (m_vertical == true)
		{
			rcRight.left = position + ui::dpi96(c_splitterSize);
			rcRight.setSize(Size(inner.getWidth() - (position + ui::dpi96(c_splitterSize) / 2) - 1, inner.getHeight()));
		}
		else
		{
			rcRight.top = position + ui::dpi96(c_splitterSize);
			rcRight.setSize(Size(inner.getWidth(), inner.getHeight() - (position + ui::dpi96(c_splitterSize) / 2) - 1));
		}
		right->setRect(rcRight);
	}
	else if (left != nullptr)
		left->setRect(inner);

	Widget::update(rc, immediate);
}

void Splitter::setPosition(int position)
{
	if (position >= 0)
	{
		m_position = position;
		update();
	}
}

int Splitter::getPosition() const
{
	return m_position;
}

Ref< Widget > Splitter::getLeftWidget() const
{
	return findVisibleSibling(getFirstChild());
}

Ref< Widget > Splitter::getRightWidget() const
{
	Widget* child = getFirstChild();
	if (child != nullptr)
		return findVisibleSibling(child->getNextSibling());
	else
		return nullptr;
}

int Splitter::getAbsolutePosition() const
{
	int position = m_position;
	if (m_relative)
		position = (m_position * (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight())) / 100;
	if (m_negative)
		position = (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight()) - position;
	return position;
}

void Splitter::setAbsolutePosition(int position)
{
	m_position = position;
	m_position = std::max< int32_t >(m_position, m_border);
	m_position = std::min< int32_t >(m_position, (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight()) - m_border);
	if (m_negative)
		m_position = (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight()) - m_position;
	if (m_relative)
		m_position = (m_position * 100) / (m_vertical ? getInnerRect().getWidth() : getInnerRect().getHeight());
}

void Splitter::eventMouseMove(MouseMoveEvent* event)
{
	Point mousePosition = event->getPosition();
	int32_t pos = m_vertical ? mousePosition.x : mousePosition.y;
	int32_t position = getAbsolutePosition();

	if (m_drag)
	{
		setAbsolutePosition(pos);
		update();
	}
	else
	{
		if (
			pos >= position - ui::dpi96(c_splitterSize) / 2 &&
			pos <= position + ui::dpi96(c_splitterSize) / 2
		)
		{
			setCursor(m_vertical ? CrSizeWE : CrSizeNS);
			setCapture();
		}
		else
		{
			resetCursor();
			releaseCapture();
		}
	}
}

void Splitter::eventButtonDown(MouseButtonDownEvent* event)
{
	Point mousePosition = event->getPosition();
	int32_t pos = m_vertical ? mousePosition.x : mousePosition.y;
	int32_t position = getAbsolutePosition();

	if (
		pos >= position - ui::dpi96(c_splitterSize) / 2 &&
		pos <= position + ui::dpi96(c_splitterSize) / 2
	)
	{
		setCursor(m_vertical ? CrSizeWE : CrSizeNS);
		setCapture();
		m_drag = true;
	}
}

void Splitter::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_drag)
	{
		releaseCapture();
		resetCursor();
		m_drag = false;
	}
}

void Splitter::eventSize(SizeEvent* event)
{
	update();
}

void Splitter::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(event->getUpdateRect());

	event->consume();
}

	}
}
