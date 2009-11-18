#include <sstream>
#include <algorithm>
#include "Ui/Custom/PropertyList/NumericPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/MouseEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

double trunc(double value)
{
	return double(long(value));
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.NumericPropertyItem", NumericPropertyItem, PropertyItem)

NumericPropertyItem::NumericPropertyItem(const std::wstring& text, double value, double limitMin, double limitMax, bool floatPoint, bool hex)
:	PropertyItem(text)
,	m_value(value)
,	m_limitMin(limitMin)
,	m_limitMax(limitMax)
,	m_floatPoint(floatPoint)
,	m_hex(hex)
,	m_mouseAdjust(false)
{
}

void NumericPropertyItem::setValue(double value)
{
	m_value = value;
}

double NumericPropertyItem::getValue() const
{
	return m_floatPoint ? m_value : trunc(m_value);
}

void NumericPropertyItem::setLimitMin(double limitMin)
{
	m_limitMin = limitMin;
	m_value = std::max< double >(m_value, m_limitMin);
}

double NumericPropertyItem::getLimitMin() const
{
	return m_limitMin;
}

void NumericPropertyItem::setLimitMax(double limitMax)
{
	m_limitMax = limitMax;
	m_value = std::min< double >(m_value, m_limitMax);
}

double NumericPropertyItem::getLimitMax() const
{
	return m_limitMax;
}

void NumericPropertyItem::setLimit(double limitMin, double limitMax)
{
	setLimitMin(limitMin);
	setLimitMax(limitMax);
}

void NumericPropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_editor);
	m_editor = new Edit();
	m_editor->create(
		parent,
		L"",
		WsNone,
		m_hex ? 0 : new NumericEditValidator(
			m_floatPoint,
			m_limitMin,
			m_limitMax
		)
	);
	m_editor->setVisible(false);
	m_editor->addFocusEventHandler(createMethodHandler(this, &NumericPropertyItem::eventEditFocus));
}

void NumericPropertyItem::destroyInPlaceControls()
{
	if (m_editor)
	{
		m_editor->destroy();
		m_editor = 0;
	}
}

void NumericPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_editor)
	{
		Rect rcEditor(rc.left, rc.top, rc.right - rc.getHeight(), rc.bottom);
		outChildRects.push_back(WidgetRect(m_editor, rcEditor));
	}
}

void NumericPropertyItem::mouseButtonDown(MouseEvent* event)
{
	if (event->getPosition().x <= m_editor->getRect().right)
	{
		std::wstringstream ss;

		double value = getValue();
		if (m_hex)
			ss << std::hex << uint32_t(value);
		else
			ss << value;

		m_editor->setText(ss.str());
		m_editor->setVisible(true);
		m_editor->setFocus();
		m_editor->selectAll();
	}
	else
	{
		m_mouseAdjust = true;
		m_mouseLastPosition = event->getPosition();
	}
}

void NumericPropertyItem::mouseButtonUp(MouseEvent* event)
{
	if (m_mouseAdjust)
	{
		m_mouseAdjust = false;
		notifyChange();
	}
}

void NumericPropertyItem::mouseMove(MouseEvent* event)
{
	if (m_mouseAdjust)
	{
		m_value += -(event->getPosition().y - m_mouseLastPosition.y) / 20.0f;
		m_value = std::max(m_value, m_limitMin);
		m_value = std::min(m_value, m_limitMax);
		m_mouseLastPosition = event->getPosition();
		notifyUpdate();
	}
}

void NumericPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	std::wstringstream ss;

	double value = getValue();
	if (m_hex)
		ss << L"0x" << std::hex << uint32_t(value);
	else
		ss << value;
	
	canvas.drawText(rc.inflate(-2, -2), ss.str(), AnLeft, AnCenter);

	int h = rc.getHeight() / 2;
	int b = h - 2;
	int x = rc.right - h;
	int y = rc.top + h;

	Point up[] =
	{
		Point(x, y - b - 1),
		Point(x + b, y - 1),
		Point(x - b, y - 1)
	};
	canvas.setBackground(value < m_limitMax ? Color(80, 80, 80) : Color(180, 180, 180));
	canvas.fillPolygon(up, 3);

	Point dw[] =
	{
		Point(x, y + b),
		Point(x - b + 1, y + 1),
		Point(x + b - 1, y + 1)
	};
	canvas.setBackground(value > m_limitMin ? Color(80, 80, 80) : Color(180, 180, 180));
	canvas.fillPolygon(dw, 3);
}

void NumericPropertyItem::eventEditFocus(Event* event)
{
	FocusEvent* f = static_cast< FocusEvent* >(event);
	if (f->lostFocus())
	{
		std::wstringstream ss(m_editor->getText());

		if (m_hex)
		{
			uint32_t value;
			ss >> std::hex >> value;
			m_value = double(value);
		}
		else
			ss >> m_value;
		
		m_editor->setVisible(false);

		notifyChange();
	}
}

		}
	}
}
