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
	namespace animation
	{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RotatorComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	enum class Axis
	{
		X,
		Y,
		Z
	};

	RotatorComponent(Axis axis, float rate);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	world::Entity* m_owner = nullptr;
	Transform m_transform = Transform::identity();
	Axis m_axis = Axis::X;
	float m_rate = 0.0f;
	float m_angle = 0.0f;

	Transform calculateLocal() const;
};

	}
}

