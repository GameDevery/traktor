#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class Play;
class Sound;

class T_DLLCLASS PlayData : public ISerializable
{
	T_RTTI_CLASS;

public:
	PlayData();

	PlayData(const resource::Id< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength);

	Ref< Play > createInstance(resource::IResourceManager* resourceManager) const;

	const resource::Id< Sound >& getSound() const { return m_sound; }

	int32_t getNote() const { return m_note; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	resource::Id< Sound > m_sound;
	int32_t m_note;
	int32_t m_repeatFrom;
	int32_t m_repeatLength;
};

	}
}
