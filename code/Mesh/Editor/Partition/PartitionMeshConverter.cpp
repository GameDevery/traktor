#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/String.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Partition/PartitionMeshConverter.h"
#include "Mesh/Partition/OctreeNodeData.h"
#include "Mesh/Partition/OctreePartitionData.h"
#include "Mesh/Partition/PartitionMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const int32_t c_maxOctreeDepth = 2;
const uint32_t c_minOctreePolygonCount = 16;

void splitPolygon(
	model::Model& model,
	const Plane& plane,
	const model::Polygon& polygon,
	model::Polygon& outBackPolygon,
	model::Polygon& outFrontPolygon
)
{
	const Scalar epsilon(FUZZY_EPSILON);

	outBackPolygon.setMaterial(polygon.getMaterial());
	outFrontPolygon.setMaterial(polygon.getMaterial());

	for (int32_t i = 0, j = (int32_t)polygon.getVertexCount() - 1; i < (int32_t)polygon.getVertexCount(); j = i++)
	{
		uint32_t ip = polygon.getVertex(j);
		uint32_t ic = polygon.getVertex(i);

		const model::Vertex& vp = model.getVertex(ip);
		const model::Vertex& vc = model.getVertex(ic);

		const Vector4& pp = model.getPosition(vp.getPosition());
		const Vector4& pc = model.getPosition(vc.getPosition());

		Scalar dp = plane.distance(pp);
		Scalar dc = plane.distance(pc);

		if ((dp < -epsilon && dc > epsilon) || (dc < -epsilon && dp > epsilon))
		{
			Scalar k;
			plane.segmentIntersection(pp, pc, k, 0);

			model::Vertex splitVertex;
			splitVertex.setPosition(model.addUniquePosition(lerp(pp, pc, k)));

			if (vp.getColor() != model::c_InvalidIndex)
			{
				const Vector4& cp = model.getColor(vp.getColor());
				const Vector4& cc = model.getColor(vc.getColor());
				splitVertex.setColor(model.addUniqueColor(lerp(cp, cc, k)));
			}
			if (vp.getNormal() != model::c_InvalidIndex)
			{
				const Vector4& np = model.getNormal(vp.getNormal());
				const Vector4& nc = model.getNormal(vc.getNormal());
				splitVertex.setNormal(model.addUniqueNormal(lerp(np, nc, k).normalized()));
			}
			if (vp.getTangent() != model::c_InvalidIndex)
			{
				const Vector4& tp = model.getNormal(vp.getTangent());
				const Vector4& tc = model.getNormal(vc.getTangent());
				splitVertex.setTangent(model.addUniqueNormal(lerp(tp, tc, k).normalized()));
			}
			if (vp.getBinormal() != model::c_InvalidIndex)
			{
				const Vector4& bp = model.getNormal(vp.getBinormal());
				const Vector4& bc = model.getNormal(vc.getBinormal());
				splitVertex.setBinormal(model.addUniqueNormal(lerp(bp, bc, k).normalized()));
			}

			for (uint32_t tci = 0; tci < vp.getTexCoordCount(); ++tci)
			{
				const Vector2& tcp = model.getTexCoord(vp.getTexCoord(tci));
				const Vector2& tcc = model.getTexCoord(vc.getTexCoord(tci));
				splitVertex.setTexCoord(tci, model.addUniqueTexCoord(lerp(tcp, tcc, k)));
			}

			uint32_t splitVertexId = model.addUniqueVertex(splitVertex);
			outBackPolygon.addVertex(splitVertexId);
			outFrontPolygon.addVertex(splitVertexId);
		}

		if (dc >= -epsilon)
			outFrontPolygon.addVertex(ic);

		if (dc <= epsilon)
			outBackPolygon.addVertex(ic);
	}

	if (outFrontPolygon.getVertexCount() < 3)
		outFrontPolygon.clearVertices();
	if (outBackPolygon.getVertexCount() < 3)
		outBackPolygon.clearVertices();
}

void splitPolygons(
	model::Model& model,
	int axis,
	float splitDistance,
	const AlignedVector< model::Polygon >& polygons,
	AlignedVector< model::Polygon >& outBackPolygons,
	AlignedVector< model::Polygon >& outFrontPolygons
)
{
	outBackPolygons.reserve((2 * polygons.size()) / 3);
	outFrontPolygons.reserve((2 * polygons.size()) / 3);
	for (AlignedVector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		float range[2] = { std::numeric_limits< float >::max(), -std::numeric_limits< float >::max() };

		for (uint32_t j = 0; j < i->getVertexCount(); ++j)
		{
			const model::Vertex& vertex = model.getVertex(i->getVertex(j));
			const Vector4& position = model.getPosition(vertex.getPosition());
			range[0] = std::min< float >(range[0], position[axis]);
			range[1] = std::max< float >(range[1], position[axis]);
		}

		if (range[0] >= splitDistance)
			outFrontPolygons.push_back(*i);
		else if (range[1] <= splitDistance)
			outBackPolygons.push_back(*i);
		else
		{
			if (std::abs(range[0] - splitDistance) > std::abs(range[1] - splitDistance))
				outFrontPolygons.push_back(*i);
			else
				outBackPolygons.push_back(*i);
		}
	}
}

struct T_ALIGN16 OctreeNodeTemplate : public Object
{
	Aabb3 boundingBox;
	AlignedVector< uint32_t > polygonIds;
	Ref< OctreeNodeTemplate > children[8];
};

Ref< OctreeNodeTemplate > buildOctreeTemplate(
	model::Model& model,
	const AlignedVector< model::Polygon >& polygons,
	int32_t depth
)
{
	Aabb3 boundingBox;
	for (AlignedVector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		for (uint32_t j = 0; j < i->getVertexCount(); ++j)
		{
			const model::Vertex& vertex = model.getVertex(i->getVertex(j));
			boundingBox.contain(model.getPosition(vertex.getPosition()));
		}
	}
	if (boundingBox.empty())
		return nullptr;

	Ref< OctreeNodeTemplate > node = new OctreeNodeTemplate();
	node->boundingBox = boundingBox;

	// Add this level's polygons to model; Ensure we triangulate polygons (since they are already simple we use a naive triangulation).
	for (AlignedVector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const model::Polygon& polygon = *i;
		if (polygon.getVertexCount() > 3)
		{
			for (uint32_t j = 0; j < polygon.getVertexCount() - 2; ++j)
			{
				model::Polygon triangle = polygon;

				triangle.clearVertices();
				triangle.addVertex(polygon.getVertex(0));
				triangle.addVertex(polygon.getVertex(1 + j));
				triangle.addVertex(polygon.getVertex(2 + j));

				node->polygonIds.push_back(model.addPolygon(triangle));
			}
		}
		else if (polygon.getVertexCount() == 3)
			node->polygonIds.push_back(model.addPolygon(*i));
	}

	if (depth >= c_maxOctreeDepth || polygons.size() <= c_minOctreePolygonCount)
		return node;

	Vector4 center = boundingBox.getCenter();

	// Split polygons into eight buckets; polygons arn't actually split
	// but instead put into "closest" bucket and instead it's bound are overlapped
	// with it's neighbor.
	AlignedVector< model::Polygon > backPolygons[3], frontPolygons[3];
	AlignedVector< model::Polygon > octPolygons[8];

	splitPolygons(model, 0, center.x(), polygons, backPolygons[0], frontPolygons[0]);

	splitPolygons(model, 1, center.y(), backPolygons[0], backPolygons[1], frontPolygons[1]);
	splitPolygons(model, 1, center.y(), frontPolygons[0], backPolygons[2], frontPolygons[2]);

	splitPolygons(model, 2, center.z(), backPolygons[1], octPolygons[0], octPolygons[4]);
	splitPolygons(model, 2, center.z(), frontPolygons[1], octPolygons[2], octPolygons[6]);
	splitPolygons(model, 2, center.z(), backPolygons[2], octPolygons[1], octPolygons[5]);
	splitPolygons(model, 2, center.z(), frontPolygons[2], octPolygons[3], octPolygons[7]);

	// Recurse with each child bucket.
	for (int32_t i = 0; i < 8; ++i)
	{
		if (!octPolygons[i].empty())
		{
			node->children[i] = buildOctreeTemplate(
				model,
				octPolygons[i],
				depth + 1
			);
		}
	}

	return node;
}

Ref< OctreeNodeData > createOctreeParts(
	const model::Model& model,
	const OctreeNodeTemplate* nodeTemplate,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	bool useLargeIndices,
	const uint8_t* indexFirst,
	uint8_t*& index,
	AlignedVector< render::Mesh::Part >& renderParts,
	AlignedVector< PartitionMeshResource::Part >& partitionParts,
	AlignedVector< std::wstring >& worldTechniques
)
{
	std::map< std::wstring, AlignedVector< IndexRange > > techniqueRanges;
	const uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	Ref< OctreeNodeData > node = new OctreeNodeData();
	node->setBoundingBox(nodeTemplate->boundingBox);

	for (std::map< std::wstring, std::list< MeshMaterialTechnique > >::const_iterator i = materialTechniqueMap.begin(); i != materialTechniqueMap.end(); ++i)
	{
		IndexRange range;

		range.offsetFirst = int32_t(index - indexFirst) / indexSize;
		range.offsetLast = 0;
		range.minIndex = std::numeric_limits< int32_t >::max();
		range.maxIndex = -std::numeric_limits< int32_t >::max();

		for (auto polygonId : nodeTemplate->polygonIds)
		{
			const model::Polygon& polygon = model.getPolygon(polygonId);
			if (model.getMaterial(polygon.getMaterial()).getName() != i->first)
				continue;

			for (int32_t k = 0; k < 3; ++k)
			{
				if (useLargeIndices)
					*(uint32_t*)index = polygon.getVertex(k);
				else
					*(uint16_t*)index = polygon.getVertex(k);

				range.minIndex = std::min< int32_t >(range.minIndex, polygon.getVertex(k));
				range.maxIndex = std::max< int32_t >(range.maxIndex, polygon.getVertex(k));

				index += indexSize;
			}
		}

		range.offsetLast = int32_t(index - indexFirst) / indexSize;
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (const auto& mmt : i->second)
		{
			std::wstring technique = mmt.worldTechnique + L"/" + mmt.shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	for (std::map< std::wstring, AlignedVector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (const auto& ir : i->second)
		{
			PartitionMeshResource::Part partitionPart;
			partitionPart.shaderTechnique = shaderTechnique;
			partitionPart.meshPart = uint32_t(renderParts.size());
			partitionPart.boundingBox = nodeTemplate->boundingBox;

			for (uint32_t k = 0; k < uint32_t(renderParts.size()); ++k)
			{
				if (
					renderParts[k].primitives.offset == ir.offsetFirst &&
					renderParts[k].primitives.count == (ir.offsetLast - ir.offsetFirst) / 3
				)
				{
					partitionPart.meshPart = k;
					break;
				}
			}

			if (partitionPart.meshPart >= renderParts.size())
			{
				render::Mesh::Part renderPart;
				renderPart.primitives.setIndexed(
					render::PtTriangles,
					ir.offsetFirst,
					(ir.offsetLast - ir.offsetFirst) / 3,
					ir.minIndex,
					ir.maxIndex
				);
				renderParts.push_back(renderPart);
			}

			auto it = std::find(worldTechniques.begin(), worldTechniques.end(), worldTechnique);
			uint8_t worldTechniqueId;

			if (it == worldTechniques.end())
			{
				worldTechniqueId = uint8_t(worldTechniques.size());
				worldTechniques.push_back(worldTechnique);
			}
			else
				worldTechniqueId = uint8_t(std::distance(worldTechniques.begin(), it));

			node->addPartIndex(worldTechniqueId, (uint32_t)partitionParts.size());
			partitionParts.push_back(partitionPart);
		}
	}

	for (int32_t i = 0; i < 8; ++i)
	{
		if (nodeTemplate->children[i])
		{
			node->setChild(i, createOctreeParts(
				model,
				nodeTemplate->children[i],
				materialTechniqueMap,
				useLargeIndices,
				indexFirst,
				index,
				renderParts,
				partitionParts,
				worldTechniques
			));
		}
	}

	return node;
}

		}

Ref< MeshResource > PartitionMeshConverter::createResource() const
{
	return new PartitionMeshResource();
}

bool PartitionMeshConverter::getOperations(const MeshAsset* meshAsset, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.push_back(new model::Triangulate());
	outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::CalculateTangents(false));
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool PartitionMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	// Create a copy of the first source model.
	model::Model model = *models[0];

	// Build octree of model; split triangles when necessary.
	log::info << L"Building octree template..." << Endl;
	AlignedVector< model::Polygon > polygons = model.getPolygons();
	model.clear(model::Model::CfPolygons | model::Model::CfJoints);
	Ref< OctreeNodeTemplate > nodeTemplate = buildOctreeTemplate(model, polygons, 0);
	T_ASSERT(nodeTemplate);

	// Create vertex declaration.
	log::info << L"Creating mesh..." << Endl;

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	bool useLargeIndices = bool(model.getVertexCount() >= 65536);
	uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	if (useLargeIndices)
		log::warning << L"Using 32-bit indices; might not work on all renderers" << Endl;

	// Create render mesh.
	uint32_t vertexBufferSize = uint32_t(model.getVertices().size() * vertexSize);
	uint32_t indexBufferSize = uint32_t(model.getPolygons().size() * 3 * indexSize);

	Ref< render::Mesh > mesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::ItUInt32 : render::ItUInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());

	for (AlignedVector< model::Vertex >::const_iterator i = model.getVertices().begin(); i != model.getVertices().end(); ++i)
	{
		std::memset(vertex, 0, vertexSize);

		writeVertexData(vertexElements, vertex, render::DuPosition, 0, model.getPosition(i->getPosition()));
		if (i->getNormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuNormal, 0, model.getNormal(i->getNormal()));
		if (i->getTangent() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuTangent, 0, model.getNormal(i->getTangent()));
		if (i->getBinormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuBinormal, 0, model.getNormal(i->getBinormal()));
		if (i->getColor() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuColor, 0, model.getColor(i->getColor()));
		if (i->getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model.getTexCoord(i->getTexCoord(0)));
		if (i->getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 1, model.getTexCoord(i->getTexCoord(1)));

		vertex += vertexSize;
	}

	mesh->getVertexBuffer()->unlock();

	// Create index buffer.
	uint8_t* index = static_cast< uint8_t* >(mesh->getIndexBuffer()->lock());
	uint8_t* indexFirst = index;

	AlignedVector< render::Mesh::Part > renderParts;
	AlignedVector< PartitionMeshResource::Part > partitionParts;

	Ref< OctreePartitionData > partitionData = new OctreePartitionData();
	partitionData->m_nodeData = createOctreeParts(
		model,
		nodeTemplate,
		materialTechniqueMap,
		useLargeIndices,
		indexFirst,
		index,
		renderParts,
		partitionParts,
		partitionData->m_worldTechniques
	);

	mesh->getIndexBuffer()->unlock();

	if (!partitionData->m_nodeData)
		return false;

	mesh->setParts(renderParts);
	mesh->setBoundingBox(model.getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, mesh))
		return false;

	checked_type_cast< PartitionMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< PartitionMeshResource* >(meshResource)->m_parts = partitionParts;
	checked_type_cast< PartitionMeshResource* >(meshResource)->m_partitionData = partitionData;

	return true;
}

	}
}
