#ifndef traktor_sound_SoundDriverNull_H
#define traktor_sound_SoundDriverNull_H

#include "Sound/SoundDriver.h"

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

/*! \brief Null device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS SoundDriverNull : public SoundDriver
{
	T_RTTI_CLASS(SoundDriverNull)

public:
	virtual bool create(const SoundDriverCreateDesc& desc);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	SoundDriverCreateDesc m_desc;
};

	}
}

#endif	// traktor_sound_SoundDriverNull_H
