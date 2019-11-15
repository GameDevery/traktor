#pragma once

#include "Core/Ref.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace physics
	{

class PhysicsManager;

	}

	namespace animation
	{

class IPoseController;
class Skeleton;

/*! Pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS IPoseControllerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) const = 0;
};

	}
}

