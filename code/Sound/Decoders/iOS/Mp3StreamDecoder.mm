/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <AudioToolbox/AudioToolbox.h>
#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/Decoders/iOS/Mp3StreamDecoder.h"

namespace traktor
{
	namespace sound
	{

class Mp3StreamDecoderImpl : public Object
{
public:
	Mp3StreamDecoderImpl()
	:	m_streamSize(0)
	{
	}

	bool create(IStream* stream)
	{
		m_stream = stream;
		m_streamSize = m_stream->available();

		OSStatus result = AudioFileOpenWithCallbacks(this, readProc, 0, getSizeProc, 0, kAudioFileMP3Type, &m_refAudioFileID);
		if (result != noErr)
		{
			log::error << L"Unable to create MP3 decoder; AudioFileOpenWithCallbacks returned error " << int32_t(result) << Endl;
			return false;
		}

		result = ExtAudioFileWrapAudioFileID(m_refAudioFileID, false, &m_inputFileID);
		if (result != noErr)
		{
			log::error << L"Unable to create MP3 decoder; ExtAudioFileWrapAudioFileID returned error " << int32_t(result) << Endl;
			return false;
		}

		AudioStreamBasicDescription clientFormat;
		std::memset(&clientFormat, 0, sizeof(clientFormat));
		clientFormat.mFormatID = kAudioFormatLinearPCM;
		clientFormat.mFramesPerPacket = 1;
		clientFormat.mChannelsPerFrame = 2;
		clientFormat.mBitsPerChannel = 16;
		clientFormat.mBytesPerPacket =
		clientFormat.mBytesPerFrame = 2 * 2;
		clientFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		clientFormat.mSampleRate = 44100;

		int size = sizeof(clientFormat);
		result = ExtAudioFileSetProperty(m_inputFileID, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat);
		if (result != noErr)
		{
			log::error << L"Unable to create MP3 decoder; (1) ExtAudioFileSetProperty returned error " << int32_t(result) << Endl;
			return false;
		}

		m_buffer.reset((int16_t*)Alloc::acquireAlign(65536 * sizeof(int16_t), 16, T_FILE_LINE));
		m_left.reset((float*)Alloc::acquireAlign(4096 * sizeof(float), 16, T_FILE_LINE));
		m_right.reset((float*)Alloc::acquireAlign(4096 * sizeof(float), 16, T_FILE_LINE));

		return true;
	}

	void destroy()
	{
		ExtAudioFileDispose(m_inputFileID);
		AudioFileClose(m_refAudioFileID);
	}

	void reset()
	{
		destroy();

		m_stream->seek(IStream::SeekSet, 0);

		create(m_stream);
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(SoundBlock& outSoundBlock)
	{
		UInt32 bufferByteSize = outSoundBlock.samplesCount * sizeof(int16_t) * 2;
		UInt32 numFrames = outSoundBlock.samplesCount;

		AudioBufferList fillBufList = { 0 };
		fillBufList.mNumberBuffers = 1;
		fillBufList.mBuffers[0].mNumberChannels = 2;
		fillBufList.mBuffers[0].mDataByteSize = bufferByteSize;
		fillBufList.mBuffers[0].mData = (void*)m_buffer.ptr();

		OSStatus result = ExtAudioFileRead(m_inputFileID, &numFrames, &fillBufList);
		if (result != noErr)
		{
			log::error << L"Unable to decode frame; ExtAudioFileRead returned error " << int32_t(result) << Endl;
			return false;
		}

		if (numFrames == 0)
			return false;

		// De-interleave buffers and convert to fp32 into left and right channels.
		const int16_t* s = m_buffer.c_ptr();

		int32_t i = 0;
		for (; i < numFrames; ++i)
		{
			m_left[i] = *s++ / 32767.0f;
			m_right[i] = *s++ / 32767.0f;
		}

		// Must produce a multiple of 4 samples thus we need to pad with mute samples,
		// which should only happen at the end of the stream.
		numFrames = alignUp(numFrames, 4);
		for (; i < numFrames; ++i)
		{
			m_left[i] = 0.0f;
			m_right[i] = 0.0f;
		}

		outSoundBlock.samples[0] = m_left.ptr();
		outSoundBlock.samples[1] = m_right.ptr();
		outSoundBlock.samplesCount = numFrames;
		outSoundBlock.sampleRate = 44100;
		outSoundBlock.maxChannel = 2;

		return true;
	}

private:
	Ref< IStream > m_stream;
	int32_t m_streamSize;
	AudioFileID m_refAudioFileID;
	ExtAudioFileRef m_inputFileID;
	AutoArrayPtr< int16_t, AllocFreeAlign > m_buffer;
	AutoArrayPtr< float, AllocFreeAlign > m_left;
	AutoArrayPtr< float, AllocFreeAlign > m_right;

	static OSStatus readProc(void* clientData, SInt64 position, UInt32 requestCount, void* buffer, UInt32* actualCount)
	{
		Mp3StreamDecoderImpl* this_ = (Mp3StreamDecoderImpl *)clientData;
		T_ASSERT (this_);

		this_->m_stream->seek(IStream::SeekSet, position);

		int32_t nread = this_->m_stream->read(buffer, requestCount);
		if (nread < 0)
			return kAudioFileEndOfFileError;

		if (actualCount)
			*actualCount = UInt32(nread);

		return noErr;
	}

	static SInt64 getSizeProc(void* clientData)
	{
		Mp3StreamDecoderImpl* this_ = (Mp3StreamDecoderImpl *)clientData;
		T_ASSERT (this_);

		return size_t(this_->m_streamSize);
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Mp3StreamDecoder", 0, Mp3StreamDecoder, IStreamDecoder)

bool Mp3StreamDecoder::create(IStream* stream)
{
	if ((m_stream = stream) == 0)
		return false;

	m_decoderImpl = new Mp3StreamDecoderImpl();
	if (!m_decoderImpl->create(m_stream))
	{
		m_decoderImpl = 0;
		m_stream = 0;
		return false;
	}

	return true;
}

void Mp3StreamDecoder::destroy()
{
	safeDestroy(m_decoderImpl);
}

double Mp3StreamDecoder::getDuration() const
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getDuration();
}

bool Mp3StreamDecoder::getBlock(SoundBlock& outSoundBlock)
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getBlock(outSoundBlock);
}

void Mp3StreamDecoder::rewind()
{
	T_ASSERT (m_decoderImpl);
	m_decoderImpl->reset();
}

	}
}
