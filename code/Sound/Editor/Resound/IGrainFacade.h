#ifndef traktor_sound_IGrainFacade_H
#define traktor_sound_IGrainFacade_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace sound
	{

class IGrainData;

class IGrainFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const IGrainData* grain) const = 0;

	virtual std::wstring getText(const IGrainData* grain) const = 0;

	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(IGrainData* parentGrain, IGrainData* childGrain) = 0;

	virtual bool removeChild(IGrainData* parentGrain, IGrainData* childGrain) = 0;

	virtual bool getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren) = 0;
};

	}
}

#endif	// traktor_sound_IGrainFacade_H
