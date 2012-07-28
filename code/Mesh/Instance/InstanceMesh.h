#ifndef traktor_mesh_InstanceMesh_H
#define traktor_mesh_InstanceMesh_H

#include "Core/Containers/SmallMap.h"
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

#define T_USE_LEGACY_INSTANCING	0

/*! \brief Instance mesh.
 *
 * Instance meshes are meshes which are repeated
 * automatically by the GPU in any number of instances
 * using hardware instancing in a single draw call.
 */
class T_DLLCLASS InstanceMesh : public IMesh
{
	T_RTTI_CLASS;

public:
#if TARGET_OS_IPHONE && T_USE_LEGACY_INSTANCING
	enum { MaxInstanceCount = 4 };		// ES doesn't support 32-bit indices thus we cannot batch enough instances.
#else
	enum { MaxInstanceCount = 20 };
#endif

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
		bool opaque;
	};

	typedef std::pair< InstanceMeshData, float > instance_distance_t;

	InstanceMesh();

	virtual ~InstanceMesh();

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;
	
	void precull(
		world::IWorldCulling* worldCulling,
		const Transform& worldTransform
	);

	void render(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		AlignedVector< instance_distance_t >& instanceWorld
	);

private:
	friend class InstanceMeshResource;

	resource::Proxy< render::Shader > m_shader;
	Ref< world::OccluderMesh > m_occluderMesh;
	Ref< render::Mesh > m_renderMesh;
	SmallMap< render::handle_t, std::vector< Part > > m_parts;
};

	}
}

#endif	// traktor_mesh_InstanceMesh_H
