#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Timer/Timer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/Tracker/IEvent.h"
#include "Sound/Tracker/Pattern.h"
#include "Sound/Tracker/Play.h"
#include "Sound/Tracker/SongBuffer.h"
#include "Sound/Tracker/Track.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const int32_t c_maxTrackCount = 16;

class SoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	float* m_outputSamples[SbcMaxChannelCount];
	Timer m_timer;
	int32_t m_bpm;
	int32_t m_currentPattern;
	int32_t m_currentRow;
	Ref< SoundChannel > m_channels[c_maxTrackCount];

	SoundBufferCursor()
	:	m_bpm(0)
	,	m_currentPattern(0)
	,	m_currentRow(-1)
	{
		m_outputSamples[0] = 0;
	}

	virtual ~SoundBufferCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter)
	{
	}

	virtual void disableRepeat()
	{
	}

	virtual void reset()
	{
		m_currentPattern = 0;
		m_currentRow = -1;
	}
};

const uint32_t c_outputSamplesBlockCount = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SongBuffer", SongBuffer, ISoundBuffer)

SongBuffer::SongBuffer(const RefArray< Pattern >& patterns, int32_t bpm)
:	m_patterns(patterns)
,	m_bpm(bpm)
{
}

Ref< ISoundBufferCursor > SongBuffer::createCursor() const
{
	Ref< SoundBufferCursor > soundBufferCursor = new SoundBufferCursor();

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	soundBufferCursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		soundBufferCursor->m_outputSamples[i] = soundBufferCursor->m_outputSamples[0] + outputSamplesCount * i;

	for (uint32_t i = 0; i < c_maxTrackCount; ++i)
		soundBufferCursor->m_channels[i] = new SoundChannel(
			i,
			44100,
			1024
		);

	soundBufferCursor->m_bpm = m_bpm;
	soundBufferCursor->m_currentPattern = 0;
	soundBufferCursor->m_currentRow = -1;
	soundBufferCursor->m_timer.start();

	return soundBufferCursor;
}

bool SongBuffer::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	SoundBufferCursor* soundBufferCursor = static_cast< SoundBufferCursor* >(cursor);
	while (soundBufferCursor->m_currentPattern < m_patterns.size())
	{
		const Pattern* currentPattern = m_patterns[soundBufferCursor->m_currentPattern];

		int32_t position = int32_t(4 * soundBufferCursor->m_bpm * soundBufferCursor->m_timer.getElapsedTime() / 60.0);
		int32_t tick = int32_t(4 * soundBufferCursor->m_bpm * soundBufferCursor->m_timer.getElapsedTime() * 2.0 / 5.0);
		
		if (position < currentPattern->getDuration())
		{
			const auto& tracks = currentPattern->getTracks();

			if (position != soundBufferCursor->m_currentRow)
			{
				soundBufferCursor->m_currentRow = position;

				for (size_t i = 0; i < tracks.size(); ++i)
				{
					const Track::Key* key = tracks[i]->findKey(position);
					if (!key)
						continue;

					auto& ch = soundBufferCursor->m_channels[i];

					if (key->play)
					{
						const auto& p = key->play;
						ch->play(
							p->getSound()->getBuffer(),
							0,
							0.0f,
							0.0f,
							0.0f,
							key->play->getRepeatLength() > 0,
							key->play->getRepeatFrom()
						);

						double pitch = std::pow(1.059463094, p->getNote() - 57);
						ch->setPitch(pitch);

						ch->setVolume(1.0f);
					}

					for (const auto& event : key->events)
						event->execute(ch, soundBufferCursor->m_bpm, soundBufferCursor->m_currentPattern, soundBufferCursor->m_currentRow);
				}
			}

			for (size_t i = 0; i < c_maxTrackCount; ++i)
			{
				auto& ch = soundBufferCursor->m_channels[i];
				if (!ch->isPlaying())
					continue;

				SoundBlock soundBlock = { { 0 }, 0, 0, 0 };
				SoundBlockMeta soundBlockMeta = { 0 };

				if (ch->getBlock(mixer, soundBlock, soundBlockMeta))
				{
					outBlock.sampleRate = max(outBlock.sampleRate, soundBlock.sampleRate);
					outBlock.samplesCount = max(outBlock.samplesCount, soundBlock.samplesCount);
					outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);
					for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
					{
						if (soundBlock.samples[j])
						{
							if (outBlock.samples[j])
							{
								mixer->addMulConst(
									outBlock.samples[j],
									soundBlock.samples[j],
									soundBlock.samplesCount,
									1.0f
								);
							}
							else
							{
								outBlock.samples[j] = soundBufferCursor->m_outputSamples[j];
								mixer->mulConst(
									outBlock.samples[j],
									soundBlock.samples[j],
									soundBlock.samplesCount,
									1.0f
								);
							}
						}
					}					
				}
			}

			break;
		}
		else
		{
			soundBufferCursor->m_currentPattern++;
			soundBufferCursor->m_currentRow = -1;
			soundBufferCursor->m_timer.start();
		}
	}
	return bool(soundBufferCursor->m_currentPattern < m_patterns.size());
}

int32_t SongBuffer::getCurrentPattern(const ISoundBufferCursor* cursor) const
{
	const SoundBufferCursor* soundBufferCursor = static_cast< const SoundBufferCursor* >(cursor);
	return soundBufferCursor->m_currentPattern;
}

int32_t SongBuffer::getCurrentRow(const ISoundBufferCursor* cursor) const
{
	const SoundBufferCursor* soundBufferCursor = static_cast< const SoundBufferCursor* >(cursor);
	return soundBufferCursor->m_currentRow;
}

	}
}
