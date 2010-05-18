#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshResource.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.IndoorMeshResource", 2, IndoorMeshResource, IMeshResource)

Ref< IMesh > IndoorMeshResource::createMesh(
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< render::Mesh > mesh = render::MeshReader(meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Indoor mesh create failed; unable to read mesh" << Endl;
		return 0;
	}

	Ref< IndoorMesh > indoorMesh = new IndoorMesh();
	indoorMesh->m_mesh = mesh;
	indoorMesh->m_shader = m_shader;

	if (!resourceManager->bind(indoorMesh->m_shader))
		return 0;

	indoorMesh->m_sectors.resize(m_sectors.size());
	for (size_t i = 0; i < m_sectors.size(); ++i)
	{
		indoorMesh->m_sectors[i].boundingBox = Aabb(m_sectors[i].min, m_sectors[i].max);

		const std::map< std::wstring, parts_t >& sectorParts = m_sectors[i].parts;
		for (std::map< std::wstring, parts_t >::const_iterator j = sectorParts.begin(); j != sectorParts.end(); ++j)
		{
			render::handle_t worldTechnique = render::getParameterHandle(j->first);

			for (parts_t::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
			{
				IndoorMesh::Part p;
				p.shaderTechnique = render::getParameterHandle(k->shaderTechnique);
				p.meshPart = k->meshPart;
				p.opaque = k->opaque;
				indoorMesh->m_sectors[i].parts[worldTechnique].push_back(p);
			}
		}
	}

	indoorMesh->m_portals.resize(m_portals.size());
	for (size_t i = 0; i < m_portals.size(); ++i)
	{
		indoorMesh->m_portals[i].winding.points = m_portals[i].pts;
		indoorMesh->m_portals[i].sectorA = m_portals[i].sectorA;
		indoorMesh->m_portals[i].sectorB = m_portals[i].sectorB;
	}

	return indoorMesh;
}

bool IndoorMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion() >= 2, L"Incorrect version");
	s >> Member< Guid >(L"shader", m_shader);
	s >> MemberAlignedVector< Sector, MemberComposite< Sector > >(L"sectors", m_sectors);
	s >> MemberAlignedVector< Portal, MemberComposite< Portal > >(L"portals", m_portals);
	return true;
}

IndoorMeshResource::Part::Part()
:	meshPart(0)
,	opaque(true)
{
}

bool IndoorMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< uint32_t >(L"meshPart", meshPart);
	s >> Member< bool >(L"opaque", opaque);
	return true;
}

bool IndoorMeshResource::Sector::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"min", min);
	s >> Member< Vector4 >(L"max", max);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		MemberStlPair<
			std::wstring,
			parts_t,
			Member< std::wstring >,
			MemberStlList< Part, MemberComposite< Part > >
		>
	>(L"parts", parts);
	return true;
}

bool IndoorMeshResource::Portal::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Vector4 >(L"pts", pts);
	s >> Member< int32_t >(L"sectorA", sectorA);
	s >> Member< int32_t >(L"sectorB", sectorB);
	return true;
}

	}
}
