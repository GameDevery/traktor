#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class IEventData;
class PlayData;
class Track;

class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Key
	{
		int32_t at;
		Ref< PlayData > play;
		RefArray< IEventData > events;

		Key();

		void serialize(ISerializer& s);
	};

	Ref< Track > createInstance(resource::IResourceManager* resourceManager) const;

	void addKey(const Key& key);

	const Key* findKey(int32_t position) const;

	const AlignedVector< Key >& getKeys() const { return m_keys; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	AlignedVector< Key > m_keys;
};

	}
}
