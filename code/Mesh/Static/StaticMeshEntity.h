#ifndef traktor_mesh_StaticMeshEntity_H
#define traktor_mesh_StaticMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

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

class StaticMesh;

class T_DLLCLASS StaticMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	StaticMeshEntity(const Transform& transform, const resource::Proxy< StaticMesh >& mesh);
	
	virtual Aabb3 getBoundingBox() const;

	virtual bool supportTechnique(render::handle_t technique) const;

	virtual void precull(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView
	);

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

private:
	resource::Proxy< StaticMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_StaticMeshEntity_H
