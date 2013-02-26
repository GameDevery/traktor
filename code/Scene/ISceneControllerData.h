#ifndef traktor_scene_ISceneControllerData_H
#define traktor_scene_ISceneControllerData_H

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityData;

	}

	namespace scene
	{

class ISceneController;

/*! \brief Scene controller data.
 * \ingroup Scene
 *
 * Scene controller data is a persistent
 * means of storing specific scene controller
 * data on disc.
 */
class T_DLLCLASS ISceneControllerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISceneController > createController(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const = 0;
};

	}
}

#endif	// traktor_scene_ISceneControllerData_H
