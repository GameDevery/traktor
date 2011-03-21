#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "World/Editor/PostProcess//PostProcessDefineCell.h"
#include "World/Editor/PostProcess//PostProcessDefineItem.h"

// Resources
#include "Resources/PostProcessDefine.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessDefineCell", PostProcessDefineCell, ui::custom::AutoWidgetCell)

PostProcessDefineCell::PostProcessDefineCell(PostProcessDefineItem* item)
:	m_item(item)
{
	m_bitmapDefine = ui::Bitmap::load(c_ResourcePostProcessDefine, sizeof(c_ResourcePostProcessDefine), L"png");
}

PostProcessDefineItem* PostProcessDefineCell::getItem() const
{
	return m_item;
}

void PostProcessDefineCell::mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	ui::CommandEvent commandEvent(widget, m_item);
	widget->raiseEvent(ui::EiSelectionChange, &commandEvent);
}

void PostProcessDefineCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	bool focus = bool(widget->getFocusCell() == this);
	canvas.drawBitmap(
		rect.getTopLeft() + ui::Size(8, 0),
		ui::Point(0, focus ? 48 : 0),
		ui::Size(48, 48),
		m_bitmapDefine,
		ui::BmAlpha
	);
	canvas.drawText(rect, m_item->getText(), ui::AnCenter, ui::AnBottom);
}

	}
}
