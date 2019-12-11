#pragma once

#include "Sound/IFilter.h"

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

/*! Reverb filter.
 * \ingroup Sound
 */
class T_DLLCLASS ReverbFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	ReverbFilter();

	ReverbFilter(
		int32_t delay,
		float duration,
		float cutOff,
		float wet
	);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_delay;
	float m_duration;
	float m_cutOff;
	float m_wet;
};

	}
}

