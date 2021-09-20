#include "Runtime/Editor/Ui/TargetInstanceListItem.h"
#include "Runtime/Editor/Ui/TargetListControl.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetListControl", TargetListControl, ui::AutoWidget)

bool TargetListControl::create(ui::Widget* parent)
{
	if (!ui::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &TargetListControl::eventButtonDown);
	return true;
}

void TargetListControl::add(TargetInstanceListItem* item)
{
	m_items.push_back(item);
	requestUpdate();
}

void TargetListControl::removeAll()
{
	m_items.resize(0);
	requestUpdate();
}

void TargetListControl::layoutCells(const ui::Rect& rc)
{
	ui::Rect targetRect = rc;
	for (auto item : m_items)
	{
		ui::Size itemSize = item->getSize();

		targetRect.bottom = targetRect.top + itemSize.cy;
		placeCell(item, targetRect);

		targetRect.top = targetRect.bottom;
	}
}

void TargetListControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	// Only allow selection with left mouse button.
	if (event->getButton() != ui::MbtLeft)
		return;

	const ui::Point& position = event->getPosition();

	TargetInstanceListItem* hitItem = dynamic_type_cast< TargetInstanceListItem* >(hitTest(position));
	if (hitItem != nullptr)
	{
		for (auto item : m_items)
			item->setSelected(item == hitItem);
	}

	ui::SelectionChangeEvent selectionChange(this);
	raiseEvent(&selectionChange);
	requestUpdate();
}

	}
}
