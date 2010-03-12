#ifndef traktor_sound_BankBuffer_H
#define traktor_sound_BankBuffer_H

#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Sound/ISoundBuffer.h"

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

class IGrain;

class T_DLLCLASS BankBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	BankBuffer(const RefArray< IGrain >& grains);

	const IGrain* getCurrentGrain(ISoundBufferCursor* cursor) const;

	void updateCursor(ISoundBufferCursor* cursor) const;

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

private:
	RefArray< IGrain > m_grains;
	mutable Semaphore m_lock;
};

	}
}

#endif	// traktor_sound_BankBuffer_H
