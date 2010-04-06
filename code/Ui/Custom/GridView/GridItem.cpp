#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/GridView/GridItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItem", GridItem, AutoWidgetCell)

GridItem::GridItem()
{
}

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
{
}

GridItem::GridItem(const std::wstring& text, Bitmap* image)
:	m_text(text)
,	m_image(image)
{
}

GridItem::GridItem(Bitmap* image)
:	m_image(image)
{
}

void GridItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& GridItem::getText() const
{
	return m_text;
}

void GridItem::setImage(Bitmap* image)
{
	m_image = image;
}

Bitmap* GridItem::getImage() const
{
	return m_image;
}

int32_t GridItem::getHeight() const
{
	int32_t height = 18;

	if (m_image)
		height = std::max(height, m_image->getSize().cy + 4);

	return height;
}

AutoWidgetCell* GridItem::hitTest(AutoWidget* widget, const Point& position)
{
	// Not allowed to pick items; entire row must be picked as selection
	// is handled by the GridView class.
	return 0;
}

void GridItem::paint(AutoWidget* widget, Canvas& canvas, const Rect& rect)
{
	Rect rcText(rect.left + 2, rect.top, rect.right, rect.bottom);

	if (m_image)
	{
		Size szImage = m_image->getSize();
		
		Point pntImage(
			rcText.left,
			rcText.top + (rcText.getHeight() - szImage.cy) / 2
		);
		if (m_text.empty())
			pntImage.x = rcText.left + (rcText.getWidth() - szImage.cx) / 2;

		canvas.drawBitmap(
			pntImage,
			Point(0, 0),
			szImage,
			m_image,
			BmAlpha
		);

		rcText.left += szImage.cx + 2;
	}

	if (!m_text.empty())
	{
		canvas.setForeground(getSystemColor(ScWindowText));
		canvas.drawText(rcText, m_text, AnLeft, AnCenter);
	}
}

		}
	}
}
