#ifndef traktor_sound_BankResource_H
#define traktor_sound_BankResource_H

#include "Core/RefArray.h"
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

class IGrainData;

/*! \brief Bank sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS BankResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	BankResource();

	BankResource(const RefArray< IGrainData >& grains, float volume);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const;

	virtual bool serialize(ISerializer& s);

	const RefArray< IGrainData >& getGrains() const { return m_grains; }

private:
	RefArray< IGrainData > m_grains;
	float m_volume;
};

	}
}

#endif	// traktor_sound_BankResource_H
