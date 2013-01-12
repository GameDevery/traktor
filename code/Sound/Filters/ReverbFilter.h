#ifndef traktor_sound_ReverbFilter_H
#define traktor_sound_ReverbFilter_H

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

/*! \brief Reverb filter.
 * \ingroup Sound
 */
class T_DLLCLASS ReverbFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	ReverbFilter();

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_samplesLength;
	int32_t m_delay;
	float m_duration;
	float m_cutOff;
	float m_wet;
};

	}
}

#endif	// traktor_sound_ReverbFilter_H
