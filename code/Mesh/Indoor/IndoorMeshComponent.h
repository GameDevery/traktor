#pragma once

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"

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

class IndoorMesh;

/*! Indoor mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS IndoorMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	IndoorMeshComponent(const resource::Proxy< IndoorMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

private:
	resource::Proxy< IndoorMesh > m_mesh;
};

	}
}

