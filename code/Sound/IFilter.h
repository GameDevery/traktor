#ifndef traktor_sound_IFilter_H
#define traktor_sound_IFilter_H

#include "Core/Serialization/ISerializable.h"
#include "Sound/Types.h"

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

class ISoundMixer;

/*! \brief Filter instance data.
* \ingroup Sound
*/
struct T_DLLCLASS IFilterInstance : public IRefCount
{
};

/*! \brief IFilter base class.
 * \ingroup Sound
 */
class T_DLLCLASS IFilter : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IFilterInstance > createInstance() const = 0;

	virtual void apply(const ISoundMixer* mixer, IFilterInstance* instance, SoundBlock& outBlock) const = 0;
};

	}
}

#endif	// traktor_sound_IFilter_H
