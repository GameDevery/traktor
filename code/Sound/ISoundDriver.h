#ifndef traktor_sound_ISoundDriver_H
#define traktor_sound_ISoundDriver_H

#include "Core/Object.h"
#include "Sound/Types.h"

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

/*! \brief Sound driver.
 * \ingroup Sound
 *
 * Sound driver works as follows:
 *  -# Sound system submits a sound block.
 *  -# Sound system waits until driver is ready for another block.
 *  -# Repeat from step 1.
 *
 * Note that even though there are no sound actually playing the
 * sound system keeps on passing "muted" sound blocks.
 *
 * A sound block passed to the driver is always of a fixed size
 * determined by the "frameSamples" member in SoundDriverCreateDesc structure.
 *
 * It the drivers responsibility to ensure that there are no glitches in the
 * playback by any means necessary.
 */
class T_DLLCLASS ISoundDriver : public Object
{
	T_RTTI_CLASS(ISoundDriver)

public:
	virtual bool create(const SoundDriverCreateDesc& desc) = 0;

	virtual void destroy() = 0;

	virtual void wait() = 0;

	virtual void submit(const SoundBlock& soundBlock) = 0;
};

	}
}

#endif	// traktor_sound_ISoundDriver_H
