#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Mesh/IMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"
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

class ProgramParameters;
class RenderContext;
class Mesh;
class Shader;

	}

	namespace world
	{

class OccluderMesh;
class IWorldCulling;
class IWorldRenderPass;

	}

	namespace mesh
	{

/*! Instance mesh.
 *
 * Instance meshes are meshes which are repeated
 * automatically by the GPU in any number of instances
 * using hardware instancing in a single draw call.
 */
class T_DLLCLASS InstanceMesh : public IMesh
{
	T_RTTI_CLASS;

public:
#if defined(__PS3__)
	enum { MaxInstanceCount = 20 };
#else
	enum { MaxInstanceCount = 60 };
#endif

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	struct RenderInstance
	{
		InstanceMeshData data;
		InstanceMeshData data0;
		float distance;

		RenderInstance()
		{
		}

		RenderInstance(const InstanceMeshData& data_, const InstanceMeshData& data0_, float distance_)
		:	data(data_)
		,	data0(data0_)
		,	distance(distance_)
		{
		}
	};

	InstanceMesh();

	virtual ~InstanceMesh();

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void getTechniques(SmallSet< render::handle_t >& outHandles) const;

	void render(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		AlignedVector< RenderInstance >& instanceWorld,
		render::ProgramParameters* extraParameters
	);

private:
	friend class InstanceMeshResource;

	resource::Proxy< render::Shader > m_shader;
	Ref< world::OccluderMesh > m_occluderMesh;
	Ref< render::Mesh > m_renderMesh;
	SmallMap< render::handle_t, AlignedVector< Part > > m_parts;
	int32_t m_maxInstanceCount;
};

	}
}

