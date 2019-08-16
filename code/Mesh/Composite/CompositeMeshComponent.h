#pragma once

#include "Core/RefArray.h"
#include "Mesh/MeshComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

/*! Composite mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS CompositeMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	CompositeMeshComponent();

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	void removeAll();

	void remove(MeshComponent* meshComponent);

	void add(MeshComponent* meshComponent);

private:
	RefArray< MeshComponent > m_meshComponents;
};

	}
}

