/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/PropertyList/PropertyContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertyContentChangeEvent", PropertyContentChangeEvent, ContentChangeEvent)

PropertyContentChangeEvent::PropertyContentChangeEvent(EventSubject* sender, PropertyItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

PropertyItem* PropertyContentChangeEvent::getItem() const
{
	return m_item;
}

	}
}
