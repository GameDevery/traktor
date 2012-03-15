#ifndef traktor_world_NullEntityData_H
#define traktor_world_NullEntityData_H

#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Null entity data.
 * \ingroup World
 */
class T_DLLCLASS NullEntityData : public EntityData
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_world_NullEntityData_H
