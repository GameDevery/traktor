#include "Mesh/MeshCulling.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.AutoLodMeshComponent", AutoLodMeshComponent, MeshComponent)

AutoLodMeshComponent::AutoLodMeshComponent(const resource::Proxy< AutoLodMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_lodDistance(0.0f)
{
}

void AutoLodMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 AutoLodMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox(m_lodDistance);
}

void AutoLodMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	Transform transform = m_transform.get(worldRenderView.getInterval());

	const Vector4& eyePosition = worldRenderView.getEyePosition();
	m_lodDistance = (transform.translation() - eyePosition).length();

	if (!m_mesh->supportTechnique(m_lodDistance, worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = m_mesh->getBoundingBox(m_lodDistance);

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

	m_mesh->build(
		m_lodDistance,
		context.getRenderContext(),
		worldRenderPass,
		m_transform.get0(),
		transform,
		distance,
		m_parameterCallback
	);
}

}
