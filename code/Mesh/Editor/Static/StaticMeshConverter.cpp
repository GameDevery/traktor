#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/String.h"
#include "Mesh/Editor/IndexRange.h"
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor
{
	namespace mesh
	{

Ref< MeshResource > StaticMeshConverter::createResource() const
{
	return new StaticMeshResource();
}

bool StaticMeshConverter::getOperations(const MeshAsset* meshAsset, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.reserve(5);
	outOperations.push_back(new model::Triangulate());
	outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::CalculateTangents(false));
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool StaticMeshConverter::convert(
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
	const model::Model* model = models[0];
	T_FATAL_ASSERT(model != nullptr);

	// Create render mesh.
	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_ASSERT(vertexSize > 0);

	bool useLargeIndices = (bool)(model->getVertexCount() >= 65536);
	uint32_t indexSize = useLargeIndices ? sizeof(uint32_t) : sizeof(uint16_t);

	// Create render mesh.
	uint32_t vertexBufferSize = (uint32_t)(model->getVertices().size() * vertexSize);
	uint32_t indexBufferSize = (uint32_t)(model->getPolygons().size() * 3 * indexSize);

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexBufferSize,
		useLargeIndices ? render::ItUInt32 : render::ItUInt16,
		indexBufferSize
	);

	// Create vertex buffer.
	uint8_t* vertex = (uint8_t*)renderMesh->getVertexBuffer()->lock();
	std::memset(vertex, 0, vertexBufferSize);

	for (const auto& v : model->getVertices())
	{
		writeVertexData(vertexElements, vertex, render::DuPosition, 0, model->getPosition(v.getPosition()));
		if (v.getNormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuNormal, 0, model->getNormal(v.getNormal()));
		if (v.getTangent() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuTangent, 0, model->getNormal(v.getTangent()));
		if (v.getBinormal() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuBinormal, 0, model->getNormal(v.getBinormal()));
		if (v.getColor() != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuColor, 0, model->getColor(v.getColor()));
		if (v.getTexCoord(0) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 0, model->getTexCoord(v.getTexCoord(0)));
		if (v.getTexCoord(1) != model::c_InvalidIndex)
			writeVertexData(vertexElements, vertex, render::DuCustom, 1, model->getTexCoord(v.getTexCoord(1)));

		vertex += vertexSize;
	}

	renderMesh->getVertexBuffer()->unlock();

	// Create index buffer.
	std::map< std::wstring, AlignedVector< IndexRange > > techniqueRanges;

	uint8_t* index = (uint8_t*)renderMesh->getIndexBuffer()->lock();
	uint8_t* indexFirst = index;

	for (const auto& mt : materialTechniqueMap)
	{
		IndexRange range;

		range.offsetFirst = uint32_t(index - indexFirst) / indexSize;
		range.offsetLast = 0;
		range.minIndex = std::numeric_limits< int32_t >::max();
		range.maxIndex = -std::numeric_limits< int32_t >::max();

		for (const auto& polygon : model->getPolygons())
		{
			T_ASSERT(polygon.getVertices().size() == 3);

			if (model->getMaterial(polygon.getMaterial()).getName() != mt.first)
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

		range.offsetLast = uint32_t(index - indexFirst) / indexSize;
		if (range.offsetLast <= range.offsetFirst)
			continue;

		for (const auto& mtt : mt.second)
		{
			std::wstring technique = mtt.worldTechnique + L"/" + mtt.shaderTechnique;
			range.mergeInto(techniqueRanges[technique]);
		}
	}

	renderMesh->getIndexBuffer()->unlock();

#if defined(_DEBUG)
	// Dump index ranges.
	log::info << L"Index ranges" << Endl;
	log::info << IncreaseIndent;
	for (const auto& tr : techniqueRanges)
	{
		log::info << L"\"" << tr.first << L"\"" << Endl;
		log::info << IncreaseIndent;
		for (uint32_t i = 0; i < tr.second.size(); ++i)
		{
			const IndexRange& range = tr.second[i];
			log::info << i << L". offset from " << range.offsetFirst << L" to " << range.offsetLast << L", index min " << range.minIndex << L" max " << range.maxIndex << Endl;
		}
		log::info << DecreaseIndent;
	}
	log::info << DecreaseIndent;
#endif

	// Build parts.
	AlignedVector< render::Mesh::Part > meshParts;
	SmallMap< std::wstring, StaticMeshResource::parts_t > parts;

	for (std::map< std::wstring, AlignedVector< IndexRange > >::const_iterator i = techniqueRanges.begin(); i != techniqueRanges.end(); ++i)
	{
		std::wstring worldTechnique, shaderTechnique;
		split(i->first, L'/', worldTechnique, shaderTechnique);

		for (AlignedVector< IndexRange >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StaticMeshResource::Part part;
			part.shaderTechnique = shaderTechnique;
			part.meshPart = uint32_t(meshParts.size());

			for (uint32_t k = 0; k < uint32_t(meshParts.size()); ++k)
			{
				if (
					meshParts[k].primitives.offset == j->offsetFirst &&
					meshParts[k].primitives.count == (j->offsetLast - j->offsetFirst) / 3
				)
				{
					part.meshPart = k;
					break;
				}
			}

			if (part.meshPart >= meshParts.size())
			{
				render::Mesh::Part meshPart;
				meshPart.primitives.setIndexed(
					render::PtTriangles,
					j->offsetFirst,
					(j->offsetLast - j->offsetFirst) / 3,
					j->minIndex,
					j->maxIndex
				);
				meshParts.push_back(meshPart);
			}

			parts[worldTechnique].push_back(part);
		}
	}

	renderMesh->setParts(meshParts);
	renderMesh->setBoundingBox(model->getBoundingBox());

	if (!render::MeshWriter().write(meshResourceStream, renderMesh))
		return false;

	checked_type_cast< StaticMeshResource* >(meshResource)->m_haveRenderMesh = true;
	checked_type_cast< StaticMeshResource* >(meshResource)->m_shader = resource::Id< render::Shader >(materialGuid);
	checked_type_cast< StaticMeshResource* >(meshResource)->m_parts = parts;
	return true;
}

	}
}
