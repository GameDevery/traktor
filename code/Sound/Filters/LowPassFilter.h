#ifndef traktor_sound_LowPassFilter_H
#define traktor_sound_LowPassFilter_H

#include "Sound/IFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Low pass filter.
 * \ingroup Sound
 */
class T_DLLCLASS LowPassFilter : public IFilter
{
	T_RTTI_CLASS(LowPassFilter)

public:
	LowPassFilter(float cutOff);

	virtual void apply(SoundBlock& outBlock);

	inline void setCutOff(float cutOff) {
		m_cutOff = cutOff;
	}

	inline float getCutOff() const {
		return m_cutOff;
	}

private:
	float m_cutOff;
	float m_history[SbcMaxChannelCount];
};

	}
}

#endif	// traktor_sound_LowPassFilter_H
