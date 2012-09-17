#ifndef traktor_sound_GroupFilter_H
#define traktor_sound_GroupFilter_H

#include "Core/RefArray.h"
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

/*! \brief Group filter.
 * \ingroup Sound
 */
class T_DLLCLASS GroupFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	GroupFilter();

	GroupFilter(IFilter* filter1, IFilter* filter2);

	void addFilter(IFilter* filter);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< IFilter > m_filters;
};

	}
}

#endif	// traktor_sound_GroupFilter_H
