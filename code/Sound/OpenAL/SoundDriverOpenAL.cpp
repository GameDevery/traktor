#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/MathUtils.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Sound/OpenAL/SoundDriverOpenAL.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

template < typename SampleType >
struct CastSample
{
	SampleType cast(float sample) const
	{
		return static_cast< SampleType >(sample * std::numeric_limits< SampleType >::max());
	}
};

#if !TARGET_OS_IPHONE

static const __m128 c_clampMin(_mm_set1_ps(-1.0f));
static const __m128 c_clampMax(_mm_set1_ps(1.0f));

template < >
struct CastSample < int8_t >
{
	__m128 f;

	CastSample()
	{
		static const float T_ALIGN16 c_int8max = std::numeric_limits< int8_t >::max();
		f = _mm_load_ss(&c_int8max);
	}

	int8_t cast(float sample) const
	{
		__m128 s = _mm_load_ss(&sample);
		s = _mm_max_ps(s, c_clampMin);
		s = _mm_min_ps(s, c_clampMax);
		__m128 sf = _mm_mul_ss(s, f);
		return (int8_t)_mm_cvtt_ss2si(sf);
	}
};

template < >
struct CastSample < int16_t >
{
	__m128 f;

	CastSample()
	{
		static const float T_ALIGN16 c_int16max = std::numeric_limits< int16_t >::max();
		f = _mm_load_ss(&c_int16max);
	}

	int16_t cast(float sample) const
	{
		__m128 s = _mm_load_ss(&sample);
		s = _mm_max_ps(s, c_clampMin);
		s = _mm_min_ps(s, c_clampMax);
		__m128 sf = _mm_mul_ss(s, f);
		return (int16_t)_mm_cvtt_ss2si(sf);
	}
};

#else

#endif

template < typename SampleType >
void writeSamples(void* dest, const float* samples, uint32_t samplesCount, uint32_t writeStride)
{
	CastSample< SampleType > cs;
	SampleType* write = static_cast< SampleType* >(dest);
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		*write = cs.cast(*samples++);
		write += writeStride;
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverOpenAL", 0, SoundDriverOpenAL, ISoundDriver)

SoundDriverOpenAL::SoundDriverOpenAL()
:	m_device(0)
,	m_context(0)
,	m_format(0)
,	m_source(0)
,	m_submitted(0)
{
	for (int i = 0; i < sizeof_array(m_buffers); ++i)
		m_buffers[i] = 0;
}

bool SoundDriverOpenAL::create(const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	const ALuint c_formats[][2] =
	{
		{ AL_FORMAT_MONO8, AL_FORMAT_STEREO8 },
		{ AL_FORMAT_MONO16, AL_FORMAT_STEREO16 }
	};

	// Open device and create context.
	m_device = alcOpenDevice(0);
	if (!m_device)
		return false;
		
	m_context = alcCreateContext(m_device, NULL);
	if (!m_context)
		return false;
		
	alcMakeContextCurrent(m_context);
	
	// Determine data format.
	int r = -1, c = -1;
	
	if (desc.bitsPerSample == 8)
		r = 0;
	else if (desc.bitsPerSample == 16)
		r = 1;
		
	if (desc.hwChannels == 1)
		c = 0;
	else if (desc.hwChannels == 2)
		c = 1;

	if (r < 0 || c < 0)
		return false;
	
	m_desc = desc;
	m_format = c_formats[r][c];
	m_submitted = 0;
	m_data.reset(
		new uint8_t [desc.frameSamples * desc.hwChannels * desc.bitsPerSample / 8]
	);
	
	// Generate buffers.
	alGenBuffers(sizeof_array(m_buffers), m_buffers);
	alGenSources(1, &m_source);
	
	alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
	alSourcef(m_source, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcei(m_source, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcei(m_source, AL_LOOPING, AL_FALSE);

	alSourcePlay(m_source);
	return true;
}

void SoundDriverOpenAL::destroy()
{
	alSourceStop(m_source);
	alDeleteSources(1, &m_source);
	alDeleteBuffers(sizeof_array(m_buffers), m_buffers);
	
	alcMakeContextCurrent(0);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

void SoundDriverOpenAL::wait()
{
	if (m_submitted < sizeof_array(m_buffers))
		return;

	// Wait until at least one buffer has been processed by OpenAL.
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	Timer timer; timer.start();
	
	while (timer.getElapsedTime() < 1.0)
	{
		ALint processed = 0;
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
		if (processed > 0)
			break;
		
		// Flush pending errors.
		alGetError();
			
		currentThread->sleep(10);
	}
}

void SoundDriverOpenAL::submit(const SoundBlock& soundBlock)
{
	ALuint buffer = 0;

	// Pop processed buffer from queue.
	if (m_submitted >= sizeof_array(m_buffers))
	{
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		if (alGetError() != AL_NO_ERROR)
		{
			log::error << L"OpenAL error detected; unable to unqueue sound block" << Endl;
			return;
		}
	}
	else
		buffer = m_buffers[m_submitted];
	
	T_ASSERT (soundBlock.maxChannel <= m_desc.hwChannels);
	T_ASSERT (soundBlock.samplesCount <= m_desc.frameSamples);
	
	if (m_desc.bitsPerSample == 8)
	{
		for (int i = 0; i < soundBlock.maxChannel; ++i)
		{
			writeSamples< uint8_t >(
				m_data.ptr() + i,
				soundBlock.samples[i],
				soundBlock.samplesCount,
				m_desc.hwChannels
			);
		}
	}
	else if (m_desc.bitsPerSample == 16)
	{
		for (int i = 0; i < soundBlock.maxChannel; ++i)
		{
			writeSamples< int16_t >(
				m_data.ptr() + i * sizeof(int16_t),
				soundBlock.samples[i],
				soundBlock.samplesCount,
				m_desc.hwChannels
			);
		}
	}
	
	// Fill buffer with sound block.
	alBufferData(
		buffer,
		m_format,
		m_data.ptr(),
		soundBlock.samplesCount * m_desc.hwChannels * m_desc.bitsPerSample / 8,
		m_desc.sampleRate
	);
	if (alGetError() != AL_NO_ERROR)
	{
		log::error << L"OpenAL error detected; unable to buffer sound block" << Endl;
		return;
	}

	// Push buffer onto queue.
	alSourceQueueBuffers(m_source, 1, &buffer);
	if (alGetError() != AL_NO_ERROR)
	{
		log::error << L"OpenAL error detected; unable to queue sound block" << Endl;
		return;
	}
	
	// Ensure source is still playing.
	ALint state = AL_PLAYING;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING)
		alSourcePlay(m_source);

	++m_submitted;
}

	}
}
