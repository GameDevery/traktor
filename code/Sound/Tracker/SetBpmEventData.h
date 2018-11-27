#pragma once

#include "Sound/Tracker/IEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SetBpmEventData : public IEventData
{
	T_RTTI_CLASS;

public:
	SetBpmEventData();

	SetBpmEventData(int32_t bpm);

	virtual Ref< IEvent > createInstance() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_bpm;
};

	}
}
