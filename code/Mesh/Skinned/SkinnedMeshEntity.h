#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Mesh/MeshEntity.h"
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

class SkinnedMesh;

class T_DLLCLASS SkinnedMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	SkinnedMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< SkinnedMesh >& mesh);

	void setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms);

	virtual Aabb3 getBoundingBox() const override final;

	virtual bool supportTechnique(render::handle_t technique) const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) override final;

private:
	resource::Proxy< SkinnedMesh > m_mesh;
	AlignedVector< Vector4 > m_jointTransforms[2];
	int32_t m_count;
};

	}
}

