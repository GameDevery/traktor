#include "Core/Timer/Timer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/MuteGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct MuteGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Timer m_timer;
	double m_end;

	virtual void setParameter(handle_t id, float parameter)  override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.MuteGrain", MuteGrain, IGrain)

MuteGrain::MuteGrain(double duration)
:	m_duration(duration)
{
}

Ref< ISoundBufferCursor > MuteGrain::createCursor() const
{
	Ref< MuteGrainCursor > muteCursor = new MuteGrainCursor();
	muteCursor->m_timer.start();
	muteCursor->m_end = muteCursor->m_timer.getElapsedTime() + m_duration;
	return muteCursor;
}

void MuteGrain::updateCursor(ISoundBufferCursor* cursor) const
{
}

const IGrain* MuteGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void MuteGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	outActiveGrains.push_back(this);
}

bool MuteGrain::getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	MuteGrainCursor* muteCursor = static_cast< MuteGrainCursor* >(cursor);
	return muteCursor->m_timer.getElapsedTime() <= muteCursor->m_end;
}

	}
}
