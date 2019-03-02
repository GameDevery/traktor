#pragma once

#include "Core/Math/TransformPath.h"
#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS PathComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	enum TimeMode
	{
		TmManual,
		TmOnce,
		TmLoop,
		TmPingPong
	};

	PathComponent(
		const TransformPath& path,
		TimeMode timeMode,
		float timeOffset
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	world::Entity* m_owner;
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
};

	}
}

