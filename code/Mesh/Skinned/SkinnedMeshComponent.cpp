#include "Core/Misc/SafeDestroy.h"
#include "Mesh/MeshCulling.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Render/Buffer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshComponent", SkinnedMeshComponent, MeshComponent)

SkinnedMeshComponent::SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, render::IRenderSystem* renderSystem, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_count(0)
{
	const auto& jointMap = m_mesh->getJointMap();
	m_jointTransforms[0] = SkinnedMesh::createJointBuffer(renderSystem, (uint32_t)jointMap.size());
	m_jointTransforms[1] = SkinnedMesh::createJointBuffer(renderSystem, (uint32_t)jointMap.size());
}

void SkinnedMeshComponent::destroy()
{
	m_mesh.clear();
	safeDestroy(m_jointTransforms[1]);
	safeDestroy(m_jointTransforms[0]);
	MeshComponent::destroy();
}

Aabb3 SkinnedMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void SkinnedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Transform worldTransform = m_transform.get(worldRenderView.getInterval());
	Transform lastWorldTransform = m_transform.get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == lastWorldTransform)
			return;
	}

	float distance = 0.0f;
	if (!isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * worldTransform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		m_jointTransforms[1 - m_count],
		m_jointTransforms[m_count],
		distance,
		m_parameterCallback
	);
}

void SkinnedMeshComponent::setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms_)
{
	render::Buffer* jointTransforms = m_jointTransforms[m_count];
	SkinnedMesh::JointData* jointData = (SkinnedMesh::JointData*)jointTransforms->lock();

	//const auto& jointMap = m_mesh->getJointMap();

	uint32_t size = (uint32_t)jointTransforms_.size();
	for (uint32_t i = 0; i < size; ++i)
	{
		Transform joint(jointTransforms_[i]);
		joint.translation().xyz1().storeAligned(jointData[i].translation);
		joint.rotation().e.storeAligned(jointData[i].rotation);
	}

	jointTransforms->unlock();

	Atomic::exchange(m_count, 1 - m_count);
}

	}
}
