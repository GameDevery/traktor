#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "World/Entity.h"

// import/export mechanism.
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

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS BoidsEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	BoidsEntity(
		const RefArray< world::Entity >& boidEntities,
		const Transform& transform,
		const Vector4& spawnPositionDiagonal,
		const Vector4& spawnVelocityDiagonal,
		const Vector4& constrain,
		float followForce,
		float repelDistance,
		float repelForce,
		float matchVelocityStrength,
		float centerForce,
		float maxVelocity
	);

	virtual ~BoidsEntity();

	virtual void destroy() override final;

	void build(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	virtual void setTransform(const Transform& transform) override final;

	virtual Transform getTransform() const override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	struct Boid
	{
		Vector4 position;
		Vector4 velocity;
	};

	RefArray< world::Entity > m_boidEntities;
	AlignedVector< Boid > m_boids;
	Transform m_transform;
	Vector4 m_constrain;
	float m_followForce;
	float m_repelDistance;
	float m_repelForce;
	float m_matchVelocityStrength;
	float m_centerForce;
	float m_maxVelocity;
};

	}
}

