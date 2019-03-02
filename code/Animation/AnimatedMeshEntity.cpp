#include <cmath>
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Functor/Functor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

#if !defined(__EMSCRIPTEN__)
//#	define T_USE_UPDATE_JOBS
#endif

namespace traktor
{
	namespace animation
	{
		namespace
		{

render::handle_t s_handleWorld_ShadowWrite;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntity", AnimatedMeshEntity, mesh::MeshEntity)

AnimatedMeshEntity::AnimatedMeshEntity(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController,
	const std::vector< int32_t >& jointRemap,
	const std::vector< Binding >& bindings,
	bool normalizePose,
	bool normalizeTransform,
	bool screenSpaceCulling
)
:	mesh::MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_jointRemap(jointRemap)
,	m_bindings(bindings)
,	m_normalizePose(normalizePose)
,	m_normalizeTransform(normalizeTransform)
,	m_index(0)
,	m_updateController(1)
,	m_updateTimeScale(1.0f)
{
	s_handleWorld_ShadowWrite = render::getParameterHandle(L"World_ShadowWrite");

	if (m_skeleton)
	{
		calculateJointTransforms(
			m_skeleton,
			m_jointTransforms
		);

		m_poseTransforms.reserve(m_jointTransforms.size());

		size_t skinJointCount = m_mesh->getJointCount();
		m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[2].resize(skinJointCount * 2, Vector4::origo());

		updatePoseController(m_index, 0.0f);

		m_index = 1 - m_index;
		m_updateController = 0;
	}
}

AnimatedMeshEntity::~AnimatedMeshEntity()
{
}

void AnimatedMeshEntity::destroy()
{
	synchronize();

	safeDestroy(m_poseController);

	for (std::vector< Binding >::iterator i = m_bindings.begin(); i != m_bindings.end(); ++i)
		safeDestroy(i->entity);
	m_bindings.clear();

	mesh::MeshEntity::destroy();
}

Aabb3 AnimatedMeshEntity::getBoundingBox() const
{
	synchronize();

	Aabb3 boundingBox = m_mesh->getBoundingBox();

	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			float radius = joint->getRadius();
			boundingBox.contain(m_poseTransforms[i].translation().xyz1(), Scalar(radius));
		}
	}

	return boundingBox;
}

bool AnimatedMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void AnimatedMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	synchronize();

	const AlignedVector< Vector4 >& skinTransforms0 = m_skinTransforms[1 - m_index];
	const AlignedVector< Vector4 >& skinTransforms1 = m_skinTransforms[m_index];

	if (skinTransforms0.size() == skinTransforms1.size())
	{
		for (uint32_t i = 0; i < skinTransforms0.size(); ++i)
			m_skinTransforms[2][i] = lerp(
				skinTransforms0[i],
				skinTransforms1[i],
				Scalar(worldRenderView.getInterval())
			);
	}

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform.get(worldRenderView.getInterval() - 1.0f),
		m_transform.get(worldRenderView.getInterval()),
		m_skinTransforms[2],
		distance,
		getParameterCallback()
	);

	for (std::vector< Binding >::iterator i = m_bindings.begin(); i != m_bindings.end(); ++i)
		worldContext.build(worldRenderView, worldRenderPass, i->entity);

	// If only entity's shadow is visible then reduce frequency of controller updates.
	if (m_updateController == 0 && worldRenderPass.getTechnique() == s_handleWorld_ShadowWrite)
	{
		m_updateController = 4;
		m_updateTimeScale = 4.0f;
	}
	else
	{
		m_updateController = 1;
		m_updateTimeScale = 1.0f;
	}
}

void AnimatedMeshEntity::update(const world::UpdateParams& update)
{
	if (m_updateController == 1)
	{
		synchronize();

		// Calculate original bone transforms in object space.
		if (m_skeleton.changed())
		{
			m_jointTransforms.resize(0);
			m_poseTransforms.resize(0);

			if (m_skeleton)
				calculateJointTransforms(
					m_skeleton,
					m_jointTransforms
				);

			m_poseTransforms.reserve(m_jointTransforms.size());
			m_skeleton.consume();
		}

		size_t skinJointCount = m_mesh->getJointCount();
		m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[2].resize(skinJointCount * 2, Vector4::origo());

		// Prevent further updates from evaluating pose controller,
		// each pose controller needs to set this flag if it's
		// required to continue running even when this entity
		// hasn't been rendered.
		m_updateController = 0;
		m_index = 1 - m_index;

#if defined(T_USE_UPDATE_JOBS)
		m_updatePoseControllerJob = JobManager::getInstance().add(makeFunctor< AnimatedMeshEntity, int32_t, float >(
			this,
			&AnimatedMeshEntity::updatePoseController,
			m_index,
			update.deltaTime
		));
#else
		updatePoseController(m_index, update.deltaTime);
#endif
	}
	else
	{
		m_index = 1 - m_index;
		m_skinTransforms[m_index] = m_skinTransforms[1 - m_index];
	}

	// Update entity to joint bindings.
	for (std::vector< Binding >::iterator i = m_bindings.begin(); i != m_bindings.end(); ++i)
	{
		Transform T;
		if (getPoseTransform(i->jointHandle, T))
			i->entity->setTransform(m_transform.get() * T);
		i->entity->update(update);
	}

	if (m_updateController > 0)
		--m_updateController;

	mesh::MeshEntity::update(update);
}

void AnimatedMeshEntity::setTransform(const Transform& transform)
{
	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);

	mesh::MeshEntity::setTransform(transform);
}

bool AnimatedMeshEntity::getJointTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	outTransform = m_jointTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getPoseTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getSkinTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_poseTransforms.size())
		return false;

	int skinIndex = m_jointRemap[index];
	if (skinIndex < 0)
		return false;

	Quaternion tmp;
	m_skinTransforms[m_index][skinIndex * 2 + 0].storeAligned((float*)&tmp);

	outTransform = Transform(
		m_skinTransforms[m_index][skinIndex * 2 + 1],
		tmp
	);
	return true;
}

bool AnimatedMeshEntity::setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive)
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	if (m_poseTransforms.empty())
		m_poseTransforms = m_jointTransforms;

	m_poseTransforms[index] = transform;

	if (inclusive)
	{
		Transform delta = transform * m_jointTransforms[index].inverse();

		std::vector< uint32_t > children;
		children.reserve(m_jointTransforms.size());

		m_skeleton->findChildren(index, children);

		for (std::vector< uint32_t >::const_iterator i = children.begin(); i != children.end(); ++i)
			m_poseTransforms[*i] = delta * m_jointTransforms[*i];
	}

	return true;
}

void AnimatedMeshEntity::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	if (m_updatePoseControllerJob)
	{
		m_updatePoseControllerJob->wait();
		m_updatePoseControllerJob = 0;
	}
#endif
}

void AnimatedMeshEntity::updatePoseController(int32_t index, float deltaTime)
{
	bool updateController = false;

	// Calculate pose transforms and skinning transforms.
	if (m_poseController)
	{
		m_poseTransforms.resize(0);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			deltaTime * m_updateTimeScale,
			m_transform.get(),
			m_skeleton,
			m_jointTransforms,
			m_poseTransforms,
			updateController
		);

		size_t skeletonJointCount = m_jointTransforms.size();
		size_t skinJointCount = m_mesh->getJointCount();

		// Ensure we have same number of pose transforms as b/ones.
		for (size_t i = m_poseTransforms.size(); i < skeletonJointCount; ++i)
			m_poseTransforms.push_back(m_jointTransforms[i]);

		if (m_normalizePose)
		{
			// Calculate pose offset in object space.
			Vector4 poseOffset = Vector4::zero();
			for (size_t i = 0; i < skeletonJointCount; ++i)
				poseOffset += m_poseTransforms[i].translation();
			poseOffset /= Scalar(float(skeletonJointCount));

			// Normalize pose transforms; update entity transform from offset.
			for (size_t i = 0; i < skeletonJointCount; ++i)
			{
				m_poseTransforms[i] = Transform(
					m_poseTransforms[i].translation() - poseOffset.xyz0(),
					m_poseTransforms[i].rotation()
				);
			}

			if (m_normalizeTransform)
				m_transform.set(Transform(
					m_transform.get().translation() + poseOffset.xyz1(),
					m_transform.get().rotation()
				));
		}

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonJointCount; ++i)
		{
			int32_t jointIndex = m_jointRemap[i];
			if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_jointTransforms[i].inverse();
				m_skinTransforms[index][jointIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[index][jointIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}
	else if (!m_poseTransforms.empty())
	{
		size_t skeletonJointCount = m_jointTransforms.size();
		size_t skinJointCount = m_mesh->getJointCount();

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonJointCount; ++i)
		{
			int32_t jointIndex = m_jointRemap[i];
			if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_jointTransforms[i].inverse();
				m_skinTransforms[index][jointIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[index][jointIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}

	if (updateController)
	{
		m_updateController = 1;
		m_updateTimeScale = 1.0f;
	}
}

	}
}
