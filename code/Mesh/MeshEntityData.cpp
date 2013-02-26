#include "Core/Serialization/ISerializer.h"
#include "Mesh/IMesh.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshEntityData", 0, MeshEntityData, AbstractMeshEntityData)

void MeshEntityData::setMesh(const resource::Id< IMesh >& mesh)
{
	m_mesh = mesh;
}

const resource::Id< IMesh >& MeshEntityData::getMesh() const
{
	return m_mesh;
}

Ref< MeshEntity > MeshEntityData::createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const
{
	resource::Proxy< IMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	Ref< MeshEntity > meshEntity;

	if (is_a< BlendMesh >(mesh.getResource()))
		meshEntity = new BlendMeshEntity(getTransform(), resource::Proxy< BlendMesh >(mesh.getHandle()));
	else if (is_a< IndoorMesh >(mesh.getResource()))
		meshEntity = new IndoorMeshEntity(getTransform(), resource::Proxy< IndoorMesh >(mesh.getHandle()));
	else if (is_a< InstanceMesh >(mesh.getResource()))
		meshEntity = new InstanceMeshEntity(getTransform(), resource::Proxy< InstanceMesh >(mesh.getHandle()));
	else if (is_a< PartitionMesh >(mesh.getResource()))
		meshEntity = new PartitionMeshEntity(getTransform(), resource::Proxy< PartitionMesh >(mesh.getHandle()));
	else if (is_a< SkinnedMesh >(mesh.getResource()))
		meshEntity = new SkinnedMeshEntity(getTransform(), resource::Proxy< SkinnedMesh >(mesh.getHandle()));
	else if (is_a< StaticMesh >(mesh.getResource()))
		meshEntity = new StaticMeshEntity(getTransform(), resource::Proxy< StaticMesh >(mesh.getHandle()));
	else if (is_a< StreamMesh >(mesh.getResource()))
		meshEntity = new StreamMeshEntity(getTransform(), resource::Proxy< StreamMesh >(mesh.getHandle()));

	return meshEntity;
}

bool MeshEntityData::serialize(ISerializer& s)
{
	if (!AbstractMeshEntityData::serialize(s))
		return false;

	return s >> resource::Member< IMesh >(L"mesh", m_mesh);
}

	}
}
