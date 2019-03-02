#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Program resource base class.
 * \ingroup Render
 */
class T_DLLCLASS ProgramResource : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

