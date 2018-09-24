/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewItemStateChangeEvent_H
#define traktor_ui_TreeViewItemStateChangeEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class TreeViewItem;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS TreeViewItemStateChangeEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	TreeViewItemStateChangeEvent(EventSubject* sender, TreeViewItem* item);

	TreeViewItem* getItem() const;

private:
	Ref< TreeViewItem > m_item;
};
	
	}
}

#endif	// traktor_ui_TreeViewItemStateChangeEvent_H
