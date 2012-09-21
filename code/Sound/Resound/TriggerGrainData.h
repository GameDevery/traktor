#ifndef traktor_sound_TriggerGrainData_H
#define traktor_sound_TriggerGrainData_H

#include "Sound/Resound/IGrainData.h"

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

class T_DLLCLASS TriggerGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	TriggerGrainData();

	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

	float getPosition() const { return m_position; }

	float getRate() const { return m_rate; }

	IGrainData* getGrain() const { return m_grain; }

private:
	float m_position;
	float m_rate;
	Ref< IGrainData > m_grain;
};

	}
}

#endif	// traktor_sound_TriggerGrainData_H
