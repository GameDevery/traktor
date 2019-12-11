#pragma once

#include "Ui/Event.h"
#include "Ui/Point.h"

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

/*! Mouse event.
 * \ingroup UI
 */
class T_DLLCLASS MouseButtonDownEvent : public Event
{
	T_RTTI_CLASS;

public:
	MouseButtonDownEvent(EventSubject* sender, int32_t button, const ui::Point& position);

	int32_t getButton() const;

	const ui::Point& getPosition() const;

private:
	int32_t m_button;
	ui::Point m_position;
};

	}
}

