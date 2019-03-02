#pragma once

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

class SequenceGroup;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS GroupVisibleEvent : public Event
{
	T_RTTI_CLASS;

public:
	GroupVisibleEvent(EventSubject* sender, SequenceGroup* group, bool visible);

	SequenceGroup* getGroup() const;

	bool getVisible() const;

private:
	Ref< SequenceGroup > m_group;
	bool m_visible;
};

	}
}

