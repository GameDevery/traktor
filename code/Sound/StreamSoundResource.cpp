#include "Core/Io/MemoryStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberType.h"
#include "Database/Instance.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/Sound.h"
#include "Sound/StreamSoundBuffer.h"
#include "Sound/StreamSoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StreamSoundResource", 2, StreamSoundResource, ISoundResource)

StreamSoundResource::StreamSoundResource()
:	m_decoderType(0)
,	m_volume(1.0f)
,	m_preload(false)
{
}

StreamSoundResource::StreamSoundResource(const TypeInfo* decoderType, float volume, bool preload)
:	m_decoderType(decoderType)
,	m_volume(volume)
,	m_preload(preload)
{
}

Ref< Sound > StreamSoundResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	Ref< IStream > stream = resourceInstance->readData(L"Data");
	if (!stream)
		return 0;

	if (!m_decoderType)
	{
		log::error << L"Unable to create sound, no decoder type" << Endl;
		return 0;
	}
	
	if (m_preload)
	{
		int32_t size = stream->available();
		
		uint8_t* buffer = new uint8_t [size];
		for (int32_t i = 0; i < size; )
		{
			int res = stream->read(&buffer[i], size - i);
			if (res <= 0)
				return 0;
			i += res;
		}

		stream = new MemoryStream(buffer, size, true, false, true);
	}
	
	Ref< IStreamDecoder > streamDecoder = checked_type_cast< IStreamDecoder* >(m_decoderType->createInstance());
	if (!streamDecoder->create(stream))
	{
		log::error << L"Unable to create sound, unable to create stream decoder" << Endl;
		return 0;
	}

	Ref< StreamSoundBuffer > soundBuffer = new StreamSoundBuffer();
	if (!soundBuffer->create(streamDecoder))
	{
		log::error << L"Unable to create sound, unable to create stream sound buffer" << Endl;
		return 0;
	}

	return new Sound(soundBuffer, m_volume);
}

bool StreamSoundResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 2);
	s >> MemberType(L"decoderType", m_decoderType);
	s >> Member< float >(L"volume", m_volume);
	s >> Member< bool >(L"preload", m_preload);
	return true;
}

	}
}
