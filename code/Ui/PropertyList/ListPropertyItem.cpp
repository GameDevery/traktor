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
#include "Ui/FloodLayout.h"
#include "Ui/ToolForm.h"
#include "Ui/MiniButton.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/PropertyList/ListPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListPropertyItem", ListPropertyItem, PropertyItem)

ListPropertyItem::ListPropertyItem(const std::wstring& text)
:	PropertyItem(text)
,	m_selected(-1)
{
}

ListPropertyItem::~ListPropertyItem()
{
	T_ASSERT(!m_buttonDrop);
	T_ASSERT(!m_listForm);
	T_ASSERT(!m_listBox);
}

int ListPropertyItem::add(const std::wstring& item)
{
	if (m_listBox)
		m_listBox->add(item);

	m_items.push_back(item);
	return int(m_items.size() - 1);
}

bool ListPropertyItem::remove(int index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	if (m_listBox)
		m_listBox->remove(index);

	std::vector< std::wstring >::iterator i = m_items.begin() + index;
	m_items.erase(i);

	if (index >= m_selected)
		m_selected = -1;

	return true;
}

void ListPropertyItem::removeAll()
{
	if (m_listBox)
		m_listBox->removeAll();

	m_items.resize(0);
	m_selected = -1;
}

int ListPropertyItem::count() const
{
	return int32_t(m_items.size());
}

std::wstring ListPropertyItem::get(int index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : L"";
}

void ListPropertyItem::select(int index)
{
	m_selected = index;
}

int ListPropertyItem::getSelected() const
{
	return m_selected;
}

std::wstring ListPropertyItem::getSelectedItem() const
{
	return get(m_selected);
}

void ListPropertyItem::createInPlaceControls(PropertyList* parent)
{
	T_ASSERT(!m_buttonDrop);
	m_buttonDrop = new MiniButton();
	m_buttonDrop->create(parent, parent->getBitmap(L"UI.SmallDots"));
	m_buttonDrop->addEventHandler< ButtonClickEvent >(this, &ListPropertyItem::eventDropClick);

	T_ASSERT(!m_listForm);
	m_listForm = new ToolForm();
	m_listForm->create(parent, L"", 0_ut, 0_ut, WsNone, new ui::FloodLayout());
	m_listForm->setVisible(false);

	T_ASSERT(!m_listBox);
	m_listBox = new ListBox();
	m_listBox->create(m_listForm, ListBox::WsSingle);
	m_listBox->addEventHandler< SelectionChangeEvent >(this, &ListPropertyItem::eventSelect);
	m_listBox->addEventHandler< FocusEvent >(this, &ListPropertyItem::eventFocus);
	m_listBox->addEventHandler< KeyDownEvent >(this, &ListPropertyItem::eventKeyDown);

	for (const auto& item : m_items)
		m_listBox->add(item);

	m_listBox->select(m_selected);
}

void ListPropertyItem::destroyInPlaceControls()
{
	if (m_buttonDrop)
	{
		m_buttonDrop->destroy();
		m_buttonDrop = nullptr;
	}

	if (m_listBox)
	{
		m_selected = m_listBox->getSelected();
		m_listBox->destroy();
		m_listBox = nullptr;
	}

	if (m_listForm)
	{
		m_listForm->destroy();
		m_listForm = nullptr;
	}
}

void ListPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_buttonDrop)
		outChildRects.push_back(WidgetRect(
			m_buttonDrop,
			Rect(
				rc.right - rc.getHeight(),
				rc.top,
				rc.right,
				rc.bottom
			)
		));

	if (m_listForm)
	{
		m_listRect = Rect(
			rc.left,
			rc.top + rc.getHeight(),
			rc.right,
			rc.top + rc.getHeight() + m_listForm->pixel(16_ut) * 8
		);
	}
}

void ListPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	std::wstring value = getSelectedItem();
	canvas.drawText(rc.inflate(-2, 0), value, AnLeft, AnCenter);
}

void ListPropertyItem::eventDropClick(ButtonClickEvent* event)
{
	if (!m_listForm->isVisible(false))
	{
		Point topLeft = m_listForm->getParent()->clientToScreen(m_listRect.getTopLeft());
		Rect listRect = m_listRect; listRect.moveTo(topLeft);

		m_listForm->setRect(listRect);
		m_listForm->setVisible(true);
		m_listBox->setFocus();
	}
	else
	{
		m_listForm->setVisible(false);
	}
}

void ListPropertyItem::eventSelect(SelectionChangeEvent* event)
{
	m_selected = m_listBox->getSelected();
	m_listForm->setVisible(false);
	notifyChange();
	notifyUpdate();
}

void ListPropertyItem::eventFocus(FocusEvent* event)
{
	if (!m_listForm->containFocus())
		m_listForm->setVisible(false);
}

void ListPropertyItem::eventKeyDown(KeyDownEvent* event)
{
	if (event->getVirtualKey() == VkEscape)
	{
		m_listForm->setVisible(false);
		event->consume();
	}
}

	}
}
