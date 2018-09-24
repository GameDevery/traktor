/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/PropertyList/PropertySelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertySelectionChangeEvent", PropertySelectionChangeEvent, SelectionChangeEvent)

PropertySelectionChangeEvent::PropertySelectionChangeEvent(EventSubject* sender, PropertyItem* item, int32_t index)
:	SelectionChangeEvent(sender)
,	m_item(item)
,	m_index(index)
{
}

PropertyItem* PropertySelectionChangeEvent::getItem() const
{
	return m_item;
}

int32_t PropertySelectionChangeEvent::getIndex() const
{
	return m_index;
}

	}
}
