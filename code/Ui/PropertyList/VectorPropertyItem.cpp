/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/PropertyList/VectorPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/MouseButtonDownEvent.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_valueWidth = 200_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.VectorPropertyItem", VectorPropertyItem, PropertyItem)

VectorPropertyItem::VectorPropertyItem(const std::wstring& text, const vector_t& value, int dimension)
:	PropertyItem(text)
,	m_dimension(dimension)
{
	T_ASSERT(m_dimension > 0);
	T_ASSERT(m_dimension <= MaxDimension);
	std::memcpy(m_value, value, sizeof(m_value));
}

void VectorPropertyItem::setValue(const vector_t& value)
{
	std::memcpy(m_value, value, sizeof(m_value));
}

const VectorPropertyItem::vector_t& VectorPropertyItem::getValue() const
{
	return m_value;
}

void VectorPropertyItem::createInPlaceControls(PropertyList* parent)
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		T_ASSERT(!m_editors[i]);
		m_editors[i] = new Edit();
		m_editors[i]->create(
			parent,
			L"",
			WsNone,
			new NumericEditValidator(
				true,
				-std::numeric_limits< float >::max(),
				std::numeric_limits< float >::max(),
				5
			)
		);
		m_editors[i]->setVisible(false);
		m_editors[i]->addEventHandler< KeyEvent >(this, &VectorPropertyItem::eventEditKey);
		m_editors[i]->addEventHandler< FocusEvent >(this, &VectorPropertyItem::eventEditFocus);
	}
}

void VectorPropertyItem::destroyInPlaceControls()
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		if (m_editors[i])
		{
			m_editors[i]->destroy();
			m_editors[i] = 0;
		}
	}
}

void VectorPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		if (!m_editors[i])
			continue;

		Rect rcSub(
			rc.left + (getPropertyList()->pixel(c_valueWidth) * i) / m_dimension,
			rc.top,
			rc.left + (getPropertyList()->pixel(c_valueWidth) * (i + 1)) / m_dimension,
			rc.bottom
		);

		outChildRects.push_back(WidgetRect(m_editors[i], rcSub));
	}
}

void VectorPropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		Rect rcSub = m_editors[i]->getRect();
		if (rcSub.inside(event->getPosition()))
		{
			m_editors[i]->setText(toString(m_value[i], 3));
			m_editors[i]->setVisible(true);
			m_editors[i]->setFocus();
			m_editors[i]->selectAll();
			break;
		}
	}
}

void VectorPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		Rect rcSub(
			rc.left + (getPropertyList()->pixel(c_valueWidth) * i) / m_dimension,
			rc.top,
			rc.left + (getPropertyList()->pixel(c_valueWidth) * (i + 1)) / m_dimension,
			rc.bottom
		);

		canvas.drawText(
			rcSub.inflate(-2, 0),
			toString(m_value[i], 3),
			AnLeft,
			AnCenter
		);
	}
}

bool VectorPropertyItem::copy()
{
	if (isEditing())
		return false;

	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	StringOutputStream ss;
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		if (i > 0)
			ss << L",";
		ss << m_value[i];
	}
	return clipboard->setText(ss.str());
}

bool VectorPropertyItem::paste()
{
	if (isEditing())
		return false;

	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	std::vector< float > values;
	Split< std::wstring, float >::any(clipboard->getText(), L",", values, true);
	if (values.size() >= m_dimension)
	{
		for (int i = 0; i < m_dimension; ++i)
			m_value[i] = values[i];
		return true;
	}
	else
		return false;
}

bool VectorPropertyItem::isEditing() const
{
	for (int32_t i = 0; i < m_dimension; ++i)
	{
		if (m_editors[i] && m_editors[i]->isVisible(false))
			return true;
	}
	return false;
}

void VectorPropertyItem::eventEditKey(KeyEvent* event)
{
	if (event->getCharacter() == L'\t')
	{
		int32_t from = -1;

		// Hide all first so focus handler doesn't mess with us later.
		for (int32_t i = 0; i < m_dimension; ++i)
		{
			if (m_editors[i]->isVisible(false))
			{
				m_value[i] = parseString< float >(m_editors[i]->getText());
				m_editors[i]->setVisible(false);
				from = i;
			}
		}

		if (from >= 0)
		{
			int32_t next = ((event->getKeyState() & KsShift) == 0) ? from + 1 : from - 1;
			if (next >= m_dimension)
				next = 0;
			else if (next < 0)
				next = m_dimension - 1;

			m_editors[next]->setText(toString(m_value[next], 3));
			m_editors[next]->setVisible(true);
			m_editors[next]->setFocus();
			m_editors[next]->selectAll();
		}

		event->consume();
	}
}

void VectorPropertyItem::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus())
	{
		for (int32_t i = 0; i < m_dimension; ++i)
		{
			if (m_editors[i]->isVisible(false))
			{
				m_value[i] = parseString< float >(m_editors[i]->getText());
				m_editors[i]->setVisible(false);
			}
		}
		notifyChange();
	}
}

}
