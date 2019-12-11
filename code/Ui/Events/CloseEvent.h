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

/*! Close event.
 * \ingroup UI
 */
class T_DLLCLASS CloseEvent : public Event
{
	T_RTTI_CLASS;

public:
	CloseEvent(EventSubject* sender);

	void cancel();

	bool cancelled() const;

private:
	bool m_cancelled;
};

	}
}

