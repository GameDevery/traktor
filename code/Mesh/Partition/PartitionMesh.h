#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Matrix44.h"
#include "Mesh/IMesh.h"
#include "Render/Shader.h"
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

class RenderContext;
class Mesh;
class ITexture;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;
class IPartition;

/*! Partition mesh.
 */
class T_DLLCLASS PartitionMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
		Aabb3 boundingBox;
	};

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void build(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class PartitionMeshResource;

	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_mesh;
	AlignedVector< Part > m_parts;
	Ref< IPartition > m_partition;
	AlignedVector< uint32_t > m_partIndices;
#if defined(_DEBUG)
	std::string m_name;
#endif
};

	}
}

