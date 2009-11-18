#ifndef traktor_sound_DitherFilter_H
#define traktor_sound_DitherFilter_H

#include "Sound/IFilter.h"
#include "Core/Math/Random.h"

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

/*! \brief Dither filter.
 * \ingroup Sound
 */
class T_DLLCLASS DitherFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	DitherFilter(uint32_t bitsPerSample = 16);

	virtual void apply(SoundBlock& outBlock);

private:
	float m_ditherAmplitude;
	Random m_random;
};

	}
}

#endif	// traktor_sound_DitherFilter_H
