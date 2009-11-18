#ifndef traktor_ISerializable_H
#define traktor_ISerializable_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

/*! \brief Base class of each serializable class.
 * \ingroup Core
 *
 * Any class supporting serialization must be
 * derived from this class.
 */
class T_DLLCLASS ISerializable : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Serialize object.
	 *
	 * \param s Serializer interface.
	 * \return True if serialization successful.
	 */
	virtual bool serialize(ISerializer& s) = 0;
};

}

#endif	// traktor_ISerializable_H
