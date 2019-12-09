#pragma once

#include <string>
#include "Runtime/IAudioServer.h"
#include "Core/Platform.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

	namespace sound
	{

class SoundPlayer;

	}

	namespace runtime
	{

class IEnvironment;

/*! \brief
 * \ingroup Runtime
 */
class AudioServer : public IAudioServer
{
	T_RTTI_CLASS;

public:
	AudioServer();

	bool create(const PropertyGroup* settings, const SystemApplication& sysapp);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float dT, bool renderViewActive);

	uint32_t getActiveSoundChannels() const;

	virtual sound::AudioSystem* getAudioSystem() override final;

	virtual sound::ISoundPlayer* getSoundPlayer() override final;

	virtual sound::SurroundEnvironment* getSurroundEnvironment() override final;

private:
	Ref< sound::AudioSystem > m_audioSystem;
	Ref< sound::SoundPlayer > m_soundPlayer;
	Ref< sound::SurroundEnvironment > m_surroundEnvironment;
	std::wstring m_audioType;
	bool m_autoMute;
	bool m_soundMuted;
	float m_soundMutedVolume;
};

	}
}

