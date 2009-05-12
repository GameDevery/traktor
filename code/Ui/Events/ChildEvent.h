#ifndef traktor_ui_ChildEvent_H
#define traktor_ui_ChildEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
	
class Widget;

/*! \brief ChildEvent
 * \ingroup UI
 */
class T_DLLCLASS ChildEvent : public Event
{
	T_RTTI_CLASS(ChildEvent)
	
public:
	ChildEvent(EventSubject* sender, Widget* child, bool link);

	bool link() const;	

private:
	bool m_link;
};
	
	}
}

#endif	// traktor_ui_ChildEvent_H
