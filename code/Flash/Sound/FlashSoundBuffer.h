#ifndef traktor_flash_FlashSoundBuffer_H
#define traktor_flash_FlashSoundBuffer_H

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace flash
	{

class FlashSound;

class FlashSoundBuffer : public sound::ISoundBuffer
{
	T_RTTI_CLASS;

public:
	FlashSoundBuffer(const FlashSound* sound);

	virtual Ref< sound::ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const;

private:
	Ref< const FlashSound > m_sound;
};

	}
}

#endif	// traktor_flash_FlashSoundBuffer_H
