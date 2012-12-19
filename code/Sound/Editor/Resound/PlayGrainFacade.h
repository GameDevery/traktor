#ifndef traktor_sound_PlayGrainFacade_H
#define traktor_sound_PlayGrainFacade_H

#include "Sound/Editor/Resound/IGrainFacade.h"

namespace traktor
{
	namespace sound
	{

class PlayGrainFacade : public IGrainFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const IGrainData* grain) const;

	virtual std::wstring getText(const IGrainData* grain) const;

	virtual bool getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const;

	virtual bool canHaveChildren() const;

	virtual bool addChild(IGrainData* parentGrain, IGrainData* childGrain);

	virtual bool removeChild(IGrainData* parentGrain, IGrainData* childGrain);

	virtual bool getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren);
};

	}
}

#endif	// traktor_sound_PlayGrainFacade_H
