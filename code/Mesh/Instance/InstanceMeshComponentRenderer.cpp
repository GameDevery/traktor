#include "Mesh/MeshCulling.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponentRenderer", InstanceMeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet InstanceMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< InstanceMeshComponent >();
}

void InstanceMeshComponentRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< const world::LightComponent* >& outLights,
	AlignedVector< const world::ProbeComponent* >& outProbes
)
{
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	InstanceMeshComponent* meshComponent = mandatory_non_null_type_cast< InstanceMeshComponent* >(renderable);
	InstanceMesh* mesh = meshComponent->getMesh();

	if (!mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = meshComponent->getBoundingBox();
	Transform transform = meshComponent->getTransform().get(worldRenderView.getInterval());

	float distance = 0.0f;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * transform.toMatrix44(),
		worldRenderView.getProjection(),
		1e-4f,
		distance
	))
		return;

	Transform transformLast = meshComponent->getTransform().get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (transform == transformLast)
			return;
	}

	T_FATAL_ASSERT(m_meshInstances[mesh].size() < InstanceMesh::MaxInstanceCount);
	m_meshInstances[mesh].push_back(InstanceMesh::RenderInstance(
		packInstanceMeshData(transform),
		packInstanceMeshData(transformLast),
		distance
	));
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	for (auto& it : m_meshInstances)
	{
		if (it.second.empty())
			continue;

		it.first->build(
			context.getRenderContext(),
			worldRenderPass,
			it.second,
			nullptr
		);

		it.second.resize(0);
	}
}

	}
}
