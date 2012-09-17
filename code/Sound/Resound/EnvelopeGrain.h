#ifndef traktor_sound_EnvelopeGrain_H
#define traktor_sound_EnvelopeGrain_H

#include "Core/RefArray.h"
#include "Sound/Resound/IGrain.h"

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

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS EnvelopeGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	struct Grain
	{
		Ref< IGrain > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;
	};

	EnvelopeGrain(const std::vector< Grain >& grains);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const;

private:
	std::vector< Grain > m_grains;
};

	}
}

#endif	// traktor_sound_EnvelopeGrain_H
