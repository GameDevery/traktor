#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/GridView/GridHeader.h"
#include "Ui/GridView/GridColumn.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridHeader", GridHeader, AutoWidgetCell)

void GridHeader::setColumns(const RefArray< GridColumn >& columns)
{
	m_columns = columns;
}

void GridHeader::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	if (m_columns.size() < 2)
		return;

	int32_t dx = dpi96(2);
	int32_t x = 0;

	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];
		x += column->getWidth();
		if (position.x >= x - dx && position.x <= x + dx)
		{
			m_resizeColumn = column;
			m_resizeWidth = column->getWidth();
			m_resizePosition = x;
			break;
		}
	}
}

void GridHeader::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	m_resizeColumn = 0;
}

void GridHeader::mouseMove(MouseMoveEvent* event, const Point& position)
{
	if (!m_resizeColumn)
		return;

	int32_t width = m_resizeWidth + position.x - m_resizePosition;
	if (width < 16)
		width = 16;

	m_resizeColumn->setWidth(width);
	getWidget< AutoWidget >()->requestUpdate();
}

void GridHeader::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getWidget< AutoWidget >()->getStyleSheet();

	canvas.setBackground(ss->getColor(getWidget< AutoWidget >(), L"header-background-color"));
	canvas.fillRect(Rect(0, rect.top, rect.getWidth(), rect.bottom));

	int32_t left = rect.left;
	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];

		int32_t width = column->getWidth();
		if (m_columns.size() == 1)
			width = rect.getWidth();

		Rect rcText(left + 2, rect.top, left + width - 2, rect.bottom);

		canvas.setClipRect(rcText);

		canvas.setForeground(ss->getColor(getWidget< AutoWidget >(), getWidget< AutoWidget >()->isEnable() ? L"color" : L"color-disabled"));
		canvas.drawText(rcText, column->getTitle(), AnLeft, AnCenter);

		canvas.resetClipRect();

		canvas.setForeground(ss->getColor(getWidget< AutoWidget >(), L"line-color"));
		canvas.drawLine(left, rect.top + 4, left, rect.bottom - 4);

		left += width;
	}

	canvas.setForeground(ss->getColor(getWidget< AutoWidget >(), L"line-color"));
	canvas.drawLine(left, rect.top + 4, left, rect.bottom - 4);
}

	}
}
