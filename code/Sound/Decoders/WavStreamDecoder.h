/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_WavStreamDecoder_H
#define traktor_sound_WavStreamDecoder_H

#include "Sound/IStreamDecoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \ingroup Sound */
//@{

#pragma pack(1)

struct WaveFormat
{
	uint16_t compression;
	uint16_t channels;
	uint32_t sampleRate;
	uint32_t averageBytesPerSecond;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
};

#pragma pack()

/*! \brief WAV stream decoder.
 */
class T_DLLCLASS WavStreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) override final;

	virtual void destroy() override final;

	virtual double getDuration() const override final;

	virtual bool getBlock(SoundBlock& outSoundBlock) override final;

	virtual void rewind() override final;

private:
	Ref< IStream > m_stream;
	WaveFormat m_format;
	float T_ALIGN16 m_samplesBuffers[2][16384];

	bool readHeader();
};

//@}

	}
}

#endif	// traktor_sound_WavStreamDecoder_H
