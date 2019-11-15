#pragma once

#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class Animation;

/*! Simple animation pose controller data.
 * \ingroup Animation
 */
class T_DLLCLASS SimpleAnimationControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	SimpleAnimationControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Animation >& getAnimation() const { return m_animation; }

private:
	resource::Id< Animation > m_animation;
	bool m_linearInterpolation;
};

	}
}

