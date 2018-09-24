/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EdgeConnectEvent_H
#define traktor_ui_EdgeConnectEvent_H

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

class Edge;

/*! \brief Node or edge selection event.
 * \ingroup UI
 */
class T_DLLCLASS EdgeConnectEvent : public Event
{
	T_RTTI_CLASS;

public:
	EdgeConnectEvent(EventSubject* sender, Edge* edge);

	Edge* getEdge() const;

private:
	Ref< Edge > m_edge;
};

	}
}

#endif	// traktor_ui_EdgeConnectEvent_H
