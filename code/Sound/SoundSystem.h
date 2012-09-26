#ifndef traktor_sound_SoundSystem_H
#define traktor_sound_SoundSystem_H

#include <list>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
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

class ISoundDriver;
class ISoundMixer;
class Sound;
class SoundChannel;

/*! \brief Sound system manager.
 * \ingroup Sound
 *
 * The SoundSystem class manages mixing sound blocks
 * from virtual channels and feeding them through the
 * submission thread into the sound driver for playback.
 */
class T_DLLCLASS SoundSystem : public Object
{
	T_RTTI_CLASS;

public:
	SoundSystem(ISoundDriver* driver);

	bool create(const SoundSystemCreateDesc& desc);

	void destroy();

	/*! \brief Set global volume.
	 *
	 * \param volume Volume (0-1)
	 */
	void setVolume(float volume);

	/*! \brief Get global volume.
	 *
	 * \return Global volume.
	 */
	float getVolume() const;

	/*! \brief Set category volume.
	 *
	 * \param volume Volume (0-1)
	 */
	void setVolume(const std::wstring& category, float volume);

	/*! \brief Get category volume.
	 *
	 * \return Category volume.
	 */
	float getVolume(const std::wstring& category) const;

	/*! \brief Set global combination matrix.
	 *
	 * [hardware channel][virtual channel]
	 *
	 * \param cm Combination matrix.
	 */
	void setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount]);

	/*! \brief Get virtual channel.
	 *
	 * \param channelId Virtual channel identifier.
	 * \return Virtual sound channel.
	 */
	Ref< SoundChannel > getChannel(uint32_t channelId);

	/*! \brief Get current mixer time. */
	double getTime() const;

	/*! \brief Query performance of each thread. */
	void getThreadPerformances(double& outMixerTime, double& outSubmitTime) const;

private:
	Ref< ISoundDriver > m_driver;
	Ref< ISoundMixer > m_mixer;
	SoundSystemCreateDesc m_desc;
	float m_volume;
	Thread* m_threadMixer;
	Thread* m_threadSubmit;
	Event m_channelFinishEvent;
	RefArray< SoundChannel > m_channels;
	AlignedVector< SoundBlock > m_requestBlocks;

	// \name Submission queue
	// \{

	Semaphore m_submitQueueLock;
	Event m_submitQueueEvent;
	Event m_submitConsumedEvent;
	std::list< SoundBlock > m_submitQueue;

	// \}
	
	// \name Mixer data blocks
	// \{

	float* m_samplesData;
	std::vector< float* > m_samplesBlocks;
	int32_t m_samplesBlockHead;
	int32_t m_samplesBlockTail;
	AlignedVector< float > m_duck[2];

	// \}

	double m_time;
	double m_mixerThreadTime;
	double m_submitThreadTime;

	void threadMixer();

	void threadSubmit();
};

	}
}

#endif	// traktor_sound_SoundSystem_H
