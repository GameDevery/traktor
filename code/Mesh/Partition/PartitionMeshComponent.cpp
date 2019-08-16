#include "Mesh/MeshCulling.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMeshComponent", PartitionMeshComponent, MeshComponent)

PartitionMeshComponent::PartitionMeshComponent(const resource::Proxy< PartitionMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
{
}

void PartitionMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 PartitionMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void PartitionMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Transform transform = m_transform.get(worldRenderView.getInterval());
	Aabb3 boundingBox = m_mesh->getBoundingBox();

	float distance = 0.0f;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * transform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderView,
		worldRenderPass,
		transform,
		distance,
		m_parameterCallback
	);
}

	}
}
