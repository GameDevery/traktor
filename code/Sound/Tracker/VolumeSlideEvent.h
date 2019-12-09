#pragma once

#include "Sound/Tracker/IEvent.h"

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

class T_DLLCLASS VolumeSlideEvent : public IEvent
{
	T_RTTI_CLASS;

public:
	VolumeSlideEvent(float amount);

	virtual bool execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const T_OVERRIDE T_FINAL;

private:
	float m_amount;
};

	}
}
