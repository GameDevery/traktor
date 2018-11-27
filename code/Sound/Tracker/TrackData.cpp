#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Tracker/IEventData.h"
#include "Sound/Tracker/Play.h"
#include "Sound/Tracker/PlayData.h"
#include "Sound/Tracker/Track.h"
#include "Sound/Tracker/TrackData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.TrackData", 0, TrackData, ISerializable)

Ref< Track > TrackData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< Track > track = new Track();
	for (auto keyData : m_keys)
	{
		Track::Key& key = track->m_keys.push_back();
		key.at = keyData.at;

		if (keyData.play)
		{
			key.play = keyData.play->createInstance(resourceManager);
			if (!key.play)
				return nullptr;
		}

		for (auto eventData : keyData.events)
		{
			Ref< IEvent > event = eventData->createInstance();
			if (!event)
				return nullptr;

			key.events.push_back(event);
		}
	}
	return track;
}

void TrackData::addKey(const Key& key)
{
	m_keys.push_back(key);
}

const TrackData::Key* TrackData::findKey(int32_t position) const
{
	for (const auto& key : m_keys)
	{
		if (key.at == position)
			return &key;
	}
	return nullptr;
}

void TrackData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

TrackData::Key::Key()
:	at(0)
{
}

void TrackData::Key::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"at", at);
	s >> MemberRef< PlayData >(L"play", play);
	s >> MemberRefArray< IEventData >(L"events", events);
}

	}
}
