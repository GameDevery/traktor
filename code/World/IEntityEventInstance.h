#pragma once

#include <functional>
#include "Core/Object.h"
#include "World/WorldTypes.h"

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

class Entity;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEventInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool update(const UpdateParams& update) = 0;

	virtual void gather(const std::function< void(Entity*) >& fn) const = 0;

	virtual void cancel(CancelType when) = 0;
};

	}
}

