#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Application active events.
 * \ingroup Amalgam
 *
 * Applications are notified with this
 * event when Amalgam is becoming activated or deactivated.
 */
class T_DLLCLASS ActiveEvent : public Object
{
	T_RTTI_CLASS;

public:
	ActiveEvent(bool activated);

	bool becameActivated() const;

private:
	bool m_activated;
};

	}
}

