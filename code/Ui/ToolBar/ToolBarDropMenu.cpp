/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarDropMenu", ToolBarDropMenu, ToolBarItem)

ToolBarDropMenu::ToolBarDropMenu(int32_t width, const std::wstring& text, bool arrow, const std::wstring& toolTip)
:	m_width(width)
,	m_text(text)
,	m_arrow(arrow)
,	m_toolTip(toolTip)
,	m_hover(false)
,	m_dropPosition(0)
{
}

int32_t ToolBarDropMenu::add(MenuItem* item)
{
	int32_t index = int32_t(m_items.size() - 1);
	m_items.push_back(item);
	return index;
}

bool ToolBarDropMenu::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	RefArray< MenuItem >::iterator i = m_items.begin() + index;
	m_items.erase(i);

	return true;
}

void ToolBarDropMenu::removeAll()
{
	m_items.resize(0);
}

int32_t ToolBarDropMenu::count() const
{
	return int32_t(m_items.size());
}

MenuItem* ToolBarDropMenu::get(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : 0;
}

bool ToolBarDropMenu::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarDropMenu::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	if (m_width > 0)
		return Size(m_width, imageHeight + dpi96(4));
	else
	{
		Size sz = toolBar->getFontMetric().getExtent(m_text);
		return Size(sz.cx + dpi96(32), imageHeight + dpi96(4));
	}
}

void ToolBarDropMenu::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar, imageWidth, imageHeight);

	int32_t sep = dpi96(14);

	Rect rcText;
	if (m_arrow)
		rcText = Rect(
			at.x + dpi96(4),
			at.y + 2,
			at.x + size.cx - sep - 2,
			at.y + size.cy - 2
		);
	else
		rcText = Rect(
			at.x + 2,
			at.y + 2,
			at.x + size.cx - 2,
			at.y + size.cy - 2
		);

	canvas.setBackground(ss->getColor(toolBar, m_hover ? L"item-background-color-dropdown-hover" : L"item-background-color-dropdown"));
	canvas.fillRect(Rect(at, size));

	if (m_arrow)
	{
		Rect rcButton(
			at.x + size.cx - sep,
			at.y + 1,
			at.x + size.cx - 1,
			at.y + size.cy - 1
		);

		canvas.setBackground(ss->getColor(toolBar, L"item-background-color-dropdown-button"));
		canvas.fillRect(rcButton);

		if (m_hover)
		{
			canvas.setForeground(ss->getColor(toolBar, L"item-color-dropdown-hover"));
			canvas.drawRect(Rect(at, size));
			canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom);
		}

		Point center = rcButton.getCenter();
		Point pnts[] =
		{
			ui::Point(center.x - dpi96(3), center.y - dpi96(1)),
			ui::Point(center.x + dpi96(2), center.y - dpi96(1)),
			ui::Point(center.x - dpi96(1), center.y + dpi96(2))
		};

		canvas.setBackground(ss->getColor(toolBar, L"item-color-dropdown-arrow"));
		canvas.fillPolygon(pnts, 3);

		m_dropPosition = rcButton.left;
	}
	else
		m_dropPosition = 0;

	canvas.setForeground(ss->getColor(toolBar, L"color"));
	canvas.drawText(rcText, m_text, m_arrow ? AnLeft : AnCenter, AnCenter);

	m_menuPosition = Point(at.x, at.y + size.cy);
}

bool ToolBarDropMenu::mouseEnter(ToolBar* toolBar)
{
	m_hover = true;
	return true;
}

void ToolBarDropMenu::mouseLeave(ToolBar* toolBar)
{
	m_hover = false;
}

void ToolBarDropMenu::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	Menu menu;

	for (size_t i = 0; i < m_items.size(); ++i)
		menu.add(m_items[i]);

	const MenuItem* item = menu.showModal(toolBar, m_menuPosition);
	if (item != nullptr)
	{
		ToolBarButtonClickEvent clickEvent(toolBar, this, item->getCommand());
		toolBar->raiseEvent(&clickEvent);
	}

	toolBar->update();
}

void ToolBarDropMenu::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

	}
}
