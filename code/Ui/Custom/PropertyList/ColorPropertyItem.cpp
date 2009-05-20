#include <sstream>
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Command.h"
#include "Ui/Events/MouseEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorPropertyItem", ColorPropertyItem, PropertyItem)

ColorPropertyItem::ColorPropertyItem(const std::wstring& text, const Color& value)
:	PropertyItem(text)
,	m_value(value)
,	m_rcColor(0, 0, 0, 0)
{
}

void ColorPropertyItem::setValue(const Color& value)
{
	m_value = value;
}

const Color& ColorPropertyItem::getValue() const
{
	return m_value;
}

void ColorPropertyItem::mouseButtonUp(MouseEvent* event)
{
	if (m_rcColor.inside(event->getPosition()))
		notifyCommand(Command(L"Property.Edit"));
}

void ColorPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	// Format color as string.
	std::wstringstream ss;
	ss << m_value.r << L", " << m_value.g << L", " << m_value.b << L", " << m_value.a;
	canvas.drawText(rc.inflate(-2, -2).offset(22, 0), ss.str(), AnLeft, AnCenter);

	// Ignore alpha when drawing color preview.
	Color previewColor = m_value;
	previewColor.a = 255;

	canvas.setBackground(previewColor);
	canvas.setForeground(Color(0, 0, 0));

	// Draw color preview with a black border.
	m_rcColor = Rect(rc.left + 2, rc.top + 2, rc.left + 20 + 2, rc.bottom - 2);
	canvas.fillRect(m_rcColor);
	canvas.drawRect(m_rcColor);
}

		}
	}
}
