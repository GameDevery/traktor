/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/ListBox/ListBoxItem.h"

namespace traktor
{
	namespace ui
	{
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListBoxItem", ListBoxItem, AutoWidgetCell)

ListBoxItem::ListBoxItem()
:	m_selected(false)
{
}

void ListBoxItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ListBoxItem::getText() const
{
	return m_text;
}

bool ListBoxItem::setSelected(bool selected)
{
	if (selected != m_selected)
	{
		m_selected = selected;
		requestWidgetUpdate();
		return true;
	}
	else
		return false;
}

bool ListBoxItem::isSelected() const
{
	return m_selected;
}

void ListBoxItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	if (m_selected)
	{
		canvas.setBackground(ss->getColor(getWidget< ListBox >(), L"item-background-color-selected"));
		canvas.fillRect(rect);
	}

	canvas.setForeground(ss->getColor(getWidget< ListBox >(), m_selected ? L"item-color-selected" : L"color"));
	canvas.drawText(rect, m_text, AnLeft, AnCenter);
}

	}
}
