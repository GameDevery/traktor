#pragma once

#include "Core/Math/Aabb3.h"
#include "Core/Math/IntervalTransform.h"
#include "Render/Types.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
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

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief Base mesh entity.
 *
 * Spatial mesh entity class.
 * Each mesh entity currently support a single user defined
 * parameter to be passed to the shader system when
 * the mesh is rendered.
 */
class T_DLLCLASS MeshEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	MeshEntity(const Transform& transform, bool screenSpaceCulling);

	virtual void update(const world::UpdateParams& update) override;

	virtual void setTransform(const Transform& transform) override;

	virtual bool getTransform(Transform& outTransform) const override final;

	virtual bool supportTechnique(render::handle_t technique) const = 0;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) = 0;

	void setParameterCallback(const IMeshParameterCallback* parameterCallback) { m_parameterCallback = parameterCallback; }

	const IMeshParameterCallback* getParameterCallback() const { return m_parameterCallback; }

	bool getScreenSpaceCulling() const { return m_screenSpaceCulling; }

protected:
	IntervalTransform m_transform;
	Ref< const IMeshParameterCallback > m_parameterCallback;
	bool m_screenSpaceCulling;
};

	}
}

