#pragma once

#include "Ui/Event.h"
#include "Ui/Point.h"
#include "Ui/Enums.h"

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

/*! Key event.
 * \ingroup UI
 */
class T_DLLCLASS KeyUpEvent : public Event
{
	T_RTTI_CLASS;

public:
	KeyUpEvent(
		EventSubject* sender,
		VirtualKey virtualKey,
		uint32_t systemKey,
		wchar_t character
	);

	VirtualKey getVirtualKey() const;

	uint32_t getSystemKey() const;

	wchar_t getCharacter() const;

private:
	VirtualKey m_virtualKey;
	uint32_t m_systemKey;
	wchar_t m_character;
};

	}
}

