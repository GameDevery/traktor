#pragma once

#include "Sound/Types.h"
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

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getId() const { return m_id; }

	float getPosition() const { return m_position; }

	float getRate() const { return m_rate; }

	IGrainData* getGrain() const { return m_grain; }

private:
	std::wstring m_id;
	float m_position;
	float m_rate;
	Ref< IGrainData > m_grain;
};

	}
}

