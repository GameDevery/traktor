#ifndef traktor_amalgam_ReconfigureEvent_H
#define traktor_amalgam_ReconfigureEvent_H

#include "Amalgam/IEvent.h"
#include "Amalgam/Types.h"

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

/*! \brief Reconfigure event.
 * \ingroup Amalgam
 *
 * Applications are notified with this
 * event when any server has been reconfigured.
 */
class T_DLLCLASS ReconfigureEvent : public IEvent
{
	T_RTTI_CLASS;

public:
	ReconfigureEvent(int32_t result);

	int32_t getResult() const;

private:
	int32_t m_result;
};

	}
}

#endif	// traktor_amalgam_ReconfigureEvent_H
