/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundDriverDs8_H
#define traktor_sound_SoundDriverDs8_H

#include "Sound/Ds8/Platform.h"
#include "Sound/ISoundDriver.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_DS8_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup DS8
 */
class T_DLLCLASS SoundDriverDs8 : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	ComRef< IDirectSound8 > m_ds;
	ComRef< IDirectSoundBuffer > m_dsBuffer;
	ComRef< IDirectSoundNotify8 > m_dsNotify;
	uint32_t m_frameSamples;
	uint32_t m_bufferSize;
	uint32_t m_bufferWrite;
	WAVEFORMATEX m_wfx;
	HANDLE m_eventNotify[3];
	DSBPOSITIONNOTIFY m_dsbpn[3];
};

	}
}

#endif	// traktor_sound_SoundDriverDs8_H
