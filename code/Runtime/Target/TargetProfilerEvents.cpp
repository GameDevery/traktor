#include "Runtime/Target/TargetProfilerEvents.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"

namespace traktor
{
	namespace runtime
	{
		namespace
		{

class MemberProfilerEvent : public MemberComplex
{
public:
	MemberProfilerEvent(const wchar_t* const name, Profiler::Event& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< uint16_t >(L"name", m_ref.name);
		s >> Member< uint32_t >(L"threadId", m_ref.threadId);
		s >> Member< uint16_t >(L"depth", m_ref.depth);
		s >> Member< double >(L"start", m_ref.start);
		s >> Member< double >(L"end", m_ref.end);
	}

private:
	Profiler::Event& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetProfilerEvents", 0, TargetProfilerEvents, ISerializable)

TargetProfilerEvents::TargetProfilerEvents()
{
}

TargetProfilerEvents::TargetProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events)
:	m_currentTime(currentTime)
,	m_events(events)
{
}

void TargetProfilerEvents::serialize(ISerializer& s)
{
	s >> Member< double >(L"currentTime", m_currentTime);
	s >> MemberAlignedVector< Profiler::Event, MemberProfilerEvent >(L"events", m_events);
}

	}
}
