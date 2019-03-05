#include "Runtime/Editor/Ui/TargetStopEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetStopEvent", TargetStopEvent, ui::Event)

TargetStopEvent::TargetStopEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
{
}

TargetInstance* TargetStopEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetStopEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

	}
}
