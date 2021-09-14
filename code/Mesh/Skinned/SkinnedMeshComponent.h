#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
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
	namespace render
	{

class Buffer;
class IRenderSystem;

	}

	namespace mesh
	{

class SkinnedMesh;

/*! Skinned mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS SkinnedMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, render::IRenderSystem* renderSystem, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	void setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms);

private:
	resource::Proxy< SkinnedMesh > m_mesh;
	Ref< render::Buffer > m_jointTransforms[2];
	std::atomic< int32_t > m_count;
};

	}
}

