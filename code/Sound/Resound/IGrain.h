#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Sound/Types.h"

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

class IAudioMixer;
class ISoundBufferCursor;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS IGrain : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISoundBufferCursor > createCursor() const = 0;

	virtual void updateCursor(ISoundBufferCursor* cursor) const = 0;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const = 0;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const = 0;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const = 0;
};

	}
}

