#ifndef traktor_sound_StreamSoundResource_H
#define traktor_sound_StreamSoundResource_H

#include "Sound/ISoundResource.h"

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

/*! \brief Stream sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	StreamSoundResource();
	
	StreamSoundResource(const TypeInfo* decoderType, float volume, float presence, bool preload);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const;

	virtual bool serialize(ISerializer& s);

private:
	const TypeInfo* m_decoderType;
	float m_volume;
	float m_presence;
	bool m_preload;
};

	}
}

#endif	// traktor_sound_StreamSoundResource_H
