/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/GridView/GridRowDoubleClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRowDoubleClickEvent", GridRowDoubleClickEvent, Event)

GridRowDoubleClickEvent::GridRowDoubleClickEvent(EventSubject* sender, GridRow* row, int32_t columnIndex)
:	Event(sender)
,	m_row(row)
,	m_columnIndex(columnIndex)
{
}

GridRow* GridRowDoubleClickEvent::getRow() const
{
	return m_row;
}

int32_t GridRowDoubleClickEvent::getColumnIndex() const
{
	return m_columnIndex;
}

	}
}
