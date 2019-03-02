#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Joint description.
 * \ingroup Physics
 */
class T_DLLCLASS JointDesc : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

