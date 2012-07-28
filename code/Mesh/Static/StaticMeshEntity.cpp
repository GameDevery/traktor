#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshEntity", StaticMeshEntity, MeshEntity)

StaticMeshEntity::StaticMeshEntity(const Transform& transform, const resource::Proxy< StaticMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

Aabb3 StaticMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool StaticMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void StaticMeshEntity::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView
)
{
	m_mesh->precull(
		worldContext.getCulling(),
		getTransform(worldRenderView.getInterval())
	);
}

void StaticMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		distance,
		getParameterCallback()
	);
}

	}
}
