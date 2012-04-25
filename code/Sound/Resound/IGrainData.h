#ifndef traktor_sound_IGrainData_H
#define traktor_sound_IGrainData_H

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

class IGrain;

class T_DLLCLASS IGrainData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const = 0;
};

	}
}

#endif	// traktor_sound_IGrainData_H
