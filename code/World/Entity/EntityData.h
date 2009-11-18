#ifndef traktor_world_EntityData_H
#define traktor_world_EntityData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Entity data.
 * \ingroup World
 *
 * Serialized data used to create runtime entities.
 * The entity data is stored separately in order to
 * save runtime memory as the data structure is
 * normally tossed away when the entity has been created.
 */
class T_DLLCLASS EntityData : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_world_EntityData_H
