#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
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

class Pattern;
class TrackData;

class T_DLLCLASS PatternData : public ISerializable
{
	T_RTTI_CLASS;

public:
	PatternData();

	PatternData(int32_t duration);

	Ref< Pattern > createInstance(resource::IResourceManager* resourceManager) const;

	int32_t getDuration() const { return m_duration; }

	void addTrack(const TrackData* track);

	const RefArray< const TrackData >& getTracks() const { return m_tracks; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_duration;
	RefArray< const TrackData > m_tracks;
};

	}
}