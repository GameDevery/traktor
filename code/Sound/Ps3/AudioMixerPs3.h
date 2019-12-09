#pragma once

#include "Sound/IAudioMixer.h"

namespace traktor
{

class SpursJobQueue;

	namespace sound
	{

/*! \brief PS3/SPU sound mixer
 * \ingroup Sound
 */
class AudioMixerPs3 : public IAudioMixer
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	virtual void mulConst(float* sb, uint32_t count, float factor) const;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const;

	virtual void stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const;

	virtual void mute(float* sb, uint32_t count) const;

	virtual void synchronize() const;

private:
	Ref< SpursJobQueue > m_jobQueue;
	Ref< IAudioMixer > m_mixer;
};

	}
}

