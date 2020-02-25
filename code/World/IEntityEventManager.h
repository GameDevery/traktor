#pragma once

#include <functional>
#include <string>
#include "Core/Object.h"
#include "Core/Math/Transform.h"
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
class EntityEventSet;
class IEntityEvent;
class IEntityEventInstance;
struct UpdateParams;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEventManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual IEntityEventInstance* raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset = Transform()) = 0;

	virtual IEntityEventInstance* raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset = Transform()) = 0;

	virtual void update(const UpdateParams& update) = 0;

	virtual void gather(const std::function< void(Entity*) >& fn) const = 0;

	virtual void cancelAll(CancelType when) = 0;
};

	}
}

