#include <cstring>
#include <limits>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include "Ai/NavMeshResource.h"
#include "Ai/Editor/NavMeshAsset.h"
#include "Ai/Editor/NavMeshPipeline.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
#include "Scene/Editor/IEntityReplicator.h"
#include "Scene/Editor/Traverser.h"
#include "Terrain/OceanComponentData.h"
#include "World/EntityData.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

const float c_oceanThreshold = 0.25f;

class BuildContext : public rcContext
{
protected:
	virtual void doLog(const rcLogCategory /*category*/, const char* msg, const int /*len*/)
	{
		T_DEBUG(mbstows(msg));
	}
};

struct NavMeshSourceModel
{
	Ref< const model::Model > model;
	Transform transform;

	NavMeshSourceModel()
	{
	}

	NavMeshSourceModel(const model::Model* model_, const Transform& transform_)
	:	model(model_)
	,	transform(transform_)
	{
	}
};

void copyUnaligned3(float out[3], const Vector4& source)
{
	out[0] = source.x();
	out[1] = source.y();
	out[2] = source.z();
}

template < typename PipelineType >
Ref< ISerializable > resolveAllExternal(PipelineType* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
				return nullptr;

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(
				pipeline,
				externalEntityData
			));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(
				pipeline,
				objectMember->get()
			));
		}
	}

	return reflection->clone();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshPipeline", 13, NavMeshPipeline, editor::DefaultPipeline)

NavMeshPipeline::NavMeshPipeline()
:	m_editor(false)
,	m_build(true)
,	m_terrainStepSize(16)
{
}

bool NavMeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_editor = settings->getProperty< bool >(L"Pipeline.TargetEditor", false);
	m_build = settings->getProperty< bool >(L"NavMeshPipeline.Build", true);
	m_terrainStepSize = settings->getProperty< int32_t >(L"NavMeshPipeline.TerrainStepSize", 16);
	return true;
}

TypeInfoSet NavMeshPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< NavMeshAsset >();
}

bool NavMeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_source, editor::PdfUse);
	return true;
}

bool NavMeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);

	if (!m_build)
		return true;

	Ref< const ISerializable > sourceData = pipelineBuilder->getObjectReadOnly(asset->m_source);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to read source data" << Endl;
		return false;
	}

	sourceData = pipelineBuilder->buildOutput(sourceInstance, sourceData);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to pipeline source data" << Endl;
		return false;
	}

	sourceData = resolveAllExternal(pipelineBuilder, sourceData);

	AlignedVector< NavMeshSourceModel > navModels;
	float oceanHeight = -std::numeric_limits< float >::max();
	bool oceanClip = false;

	scene::Traverser::visit(sourceData, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
	{
		Ref< model::Model > model;

		if (auto layerEntityData = dynamic_type_cast< const world::LayerEntityData* >(entityData))
		{
			if (layerEntityData->isDynamic() || !layerEntityData->isInclude())
				return scene::Traverser::VrSkip;
		}
		else
		{
			for (auto componentData : entityData->getComponents())
			{
				// Find model synthesizer which can generate from current component.
				Ref< const scene::IEntityReplicator > entityReplicator = scene::IEntityReplicator::createEntityReplicator(type_of(componentData));
				if (entityReplicator)
					model = entityReplicator->createModel(pipelineBuilder, m_assetPath, componentData);
			}

			if (!model)
			{
				// Find model synthesizer which can generate from current entity.
				Ref< const scene::IEntityReplicator > entityReplicator = scene::IEntityReplicator::createEntityReplicator(type_of(entityData));
				if (entityReplicator)
					model = entityReplicator->createModel(pipelineBuilder, m_assetPath, entityData);
			}	

			// Explicitly check for ocean component, need to discard everything below ocean level.
			if (auto oceanComponentData = entityData->getComponent< terrain::OceanComponentData >())
			{
				oceanHeight = max< float >(oceanHeight, entityData->getTransform().translation().y());
				oceanClip = true;
			}			
		}

		if (model)
		{
			model::Triangulate().apply(*model);
			navModels.push_back(NavMeshSourceModel(
				model,
				entityData->getTransform()
			));
		}

		return scene::Traverser::VrContinue;
	});

	// Calculate aabb and count.
	Aabb3 navModelsAabb;
	uint32_t navModelsTriangleCount = 0;

	for (uint32_t i = 0; i < navModels.size(); ++i)
	{
		const model::Model* navModel = navModels[i].model;
		T_ASSERT(navModel);

		navModelsAabb.contain(navModel->getBoundingBox().transform(navModels[i].transform));
		navModelsTriangleCount += navModel->getPolygonCount();
	}

	log::info << L"\t" << navModelsTriangleCount << L" triangle(s) loaded" << Endl;
	log::info << L"Generating navigation mesh..." << Endl;

	BuildContext ctx;
	rcConfig cfg;

	std::memset(&cfg, 0, sizeof(cfg));
	cfg.cs = asset->m_cellSize;
	cfg.ch = asset->m_cellHeight;
	cfg.walkableSlopeAngle = asset->m_agentSlope;
	cfg.walkableHeight = int(std::ceil(asset->m_agentHeight / cfg.ch));
	cfg.walkableClimb = int(std::floor(asset->m_agentClimb / cfg.ch));
	cfg.walkableRadius = int(std::ceil(asset->m_agentRadius / cfg.cs));
	cfg.maxEdgeLen = int(asset->m_maxEdgeLength / asset->m_cellSize);
	cfg.maxSimplificationError = asset->m_maxSimplificationError;
	cfg.minRegionArea = int(asset->m_minRegionSize * asset->m_minRegionSize);
	cfg.mergeRegionArea = int(asset->m_mergeRegionSize * asset->m_mergeRegionSize);
	cfg.maxVertsPerPoly = 6;
	cfg.detailSampleDist = (asset->m_detailSampleDistance < 0.9f) ? 0.0f : asset->m_cellSize * asset->m_detailSampleDistance;
	cfg.detailSampleMaxError = asset->m_cellHeight * asset->m_detailSampleMaxError;

	copyUnaligned3(cfg.bmin, navModelsAabb.mn);
	copyUnaligned3(cfg.bmax, navModelsAabb.mx);

	rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

	log::info << L"NavMesh heightfield size " << cfg.width << L" * " << cfg.height << Endl;

	rcHeightfield* solid = rcAllocHeightfield();
	if (!solid)
	{
		log::error << L"NavMesh pipeline failed; unable to allocate Recast heightfield" << Endl;
		return false;
	}

	if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
	{
		log::error << L"NavMesh pipeline failed; unable to create Recast heightfield" << Endl;
		return false;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	AutoArrayPtr< uint8_t > triAreas(new uint8_t [navModelsTriangleCount]);
	if (!triAreas.c_ptr())
	{
		log::error << L"NavMesh pipeline failed; unable to memory" << Endl;
		return false;
	}

	std::memset(triAreas.ptr(), 0, navModelsTriangleCount * sizeof(uint8_t));

	{
		AlignedVector< int32_t > indices;

		uint8_t* triAreaPtr = triAreas.ptr();
		for (auto& navModel : navModels)
		{
			int32_t vertexCount = navModel.model->getVertexCount();
			int32_t triangleCount = navModel.model->getPolygonCount();

			AutoArrayPtr< float > vertices(new float [3 * vertexCount]);
			for (int32_t j = 0; j < vertexCount; ++j)
			{
				const Vector4& position = navModel.model->getVertexPosition(j);
				copyUnaligned3(&vertices[j * 3], navModel.transform * position.xyz1());
			}

			indices.resize(0);
			indices.reserve(3 * triangleCount);

			for (int32_t j = 0; j < triangleCount; ++j)
			{
				const model::Polygon& triangle = navModel.model->getPolygon(j);
				T_ASSERT(triangle.getVertexCount() == 3);

				if (oceanClip)
				{
					if (vertices[triangle.getVertex(0) * 3 + 1] < oceanHeight - c_oceanThreshold)
						continue;
					if (vertices[triangle.getVertex(1) * 3 + 1] < oceanHeight - c_oceanThreshold)
						continue;
					if (vertices[triangle.getVertex(2) * 3 + 1] < oceanHeight - c_oceanThreshold)
						continue;
				}

				indices.push_back(triangle.getVertex(2));
				indices.push_back(triangle.getVertex(1));
				indices.push_back(triangle.getVertex(0));
			}

			T_ASSERT(indices.size() / 3 <= triangleCount);

			if (!indices.empty())
			{
				rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertices.c_ptr(), vertexCount, &indices[0], indices.size() / 3, triAreaPtr);
				rcRasterizeTriangles(&ctx, vertices.c_ptr(), vertexCount, &indices[0], triAreaPtr, indices.size() / 3, *solid, cfg.walkableClimb);
			}

			navModel.model = nullptr;

			triAreaPtr += triangleCount;
		}
	}

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
	rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
	rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbors
	// between walkable cells will be calculated.
	rcCompactHeightfield* chf = rcAllocCompactHeightfield();
	if (!chf)
	{
		log::error << L"NavMesh pipeline failed; unable to allocate Recast compact heightfield" << Endl;
		return false;
	}

	if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
	{
		log::error << L"NavMesh pipeline failed; unable to build Recast compact heightfield" << Endl;
		return false;
	}

	rcFreeHeightField(solid);
	solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf))
	{
		log::error << L"NavMesh pipeline failed; unable to erode Recast walkable area" << Endl;
		return false;
	}

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *chf);

	const bool c_monotonePartitioning = false;
	if (c_monotonePartitioning)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distance field.
		if (!rcBuildRegionsMonotone(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			log::error << L"NavMesh pipeline failed; unable to build region monotones" << Endl;
			return false;
		}
	}
	else
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(&ctx, *chf))
		{
			log::error << L"NavMesh pipeline failed; unable to build distance field" << Endl;
			return false;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			log::error << L"NavMesh pipeline failed; unable to build regions" << Endl;
			return false;
		}
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	rcContourSet* cset = rcAllocContourSet();
	if (!cset)
	{
		log::error << L"NavMesh pipeline failed; unable to allocate Recast contour set" << Endl;
		return false;
	}

	if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
	{
		log::error << L"NavMesh pipeline failed; unable to build Recast contours" << Endl;
		return false;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	rcPolyMesh* pmesh = rcAllocPolyMesh();
	if (!pmesh)
	{
		log::error << L"NavMesh pipeline failed; unable to allocate Recast polygon mesh" << Endl;
		return false;
	}

	if (!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
	{
		log::error << L"NavMesh pipeline failed; unable to build Recast polygon mesh" << Endl;
		return false;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
	if (!dmesh)
	{
		log::error << L"NavMesh pipeline failed; unable to allocate Recast polygon detail mesh" << Endl;
		return false;
	}

	if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
	{
		log::error << L"NavMesh pipeline failed; unable to build Recast polygon detail mesh" << Endl;
		return false;
	}

	rcFreeCompactHeightfield(chf);
	chf = 0;

	rcFreeContourSet(cset);
	cset = 0;

	//
	// Step 8. Create Detour navigation mesh.
	//

	for (int i = 0; i < pmesh->npolys; ++i)
	{
		if (pmesh->areas[i] == RC_WALKABLE_AREA)
			pmesh->flags[i] = 0xffff;
	}

	dtNavMeshCreateParams params;
	std::memset(&params, 0, sizeof(params));

	params.verts = pmesh->verts;
	params.vertCount = pmesh->nverts;
	params.polys = pmesh->polys;
	params.polyAreas = pmesh->areas;
	params.polyFlags = pmesh->flags;
	params.polyCount = pmesh->npolys;
	params.nvp = pmesh->nvp;
	params.detailMeshes = dmesh->meshes;
	params.detailVerts = dmesh->verts;
	params.detailVertsCount = dmesh->nverts;
	params.detailTris = dmesh->tris;
	params.detailTriCount = dmesh->ntris;
	params.walkableHeight = asset->m_agentHeight;
	params.walkableRadius = asset->m_agentRadius;
	params.walkableClimb = asset->m_agentClimb;
	rcVcopy(params.bmin, pmesh->bmin);
	rcVcopy(params.bmax, pmesh->bmax);
	params.cs = cfg.cs;
	params.ch = cfg.ch;
	params.buildBvTree = false;

	uint8_t* navData = nullptr;
	int32_t navDataSize = 0;
	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		log::error << L"NavMesh pipeline failed; unable to create Detour navigation mesh data" << Endl;
		return false;
	}

	// Save navigation data in resource.
	Ref< NavMeshResource > outputResource = new NavMeshResource();

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"NavMesh pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"NavMesh pipeline failed; unable to create data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	Writer w(stream);

	w << uint8_t(2);
	w << navDataSize;

	if (stream->write(navData, navDataSize) != navDataSize)
	{
		log::error << L"NavMesh pipeline failed; unable to write to data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	// Append geometry last in NavMesh resource; currently useful for editor
	// but might come in handy later.
	w << m_editor;
	if (m_editor)
	{
		w << uint32_t(pmesh->nverts);
		for (int32_t i = 0; i < pmesh->nverts; ++i)
		{
			w << pmesh->bmin[0] + pmesh->verts[i * 3 + 0] * pmesh->cs;
			w << pmesh->bmin[1] + pmesh->verts[i * 3 + 1] * pmesh->ch;
			w << pmesh->bmin[2] + pmesh->verts[i * 3 + 2] * pmesh->cs;
		}

		w << uint32_t(pmesh->npolys);
		for (int32_t i = 0; i < pmesh->npolys; ++i)
		{
			const uint16_t* p = &pmesh->polys[i * pmesh->nvp * 2];

			uint32_t nvp = 0;
			for (; nvp < pmesh->nvp; ++nvp)
			{
				if (p[nvp] == RC_MESH_NULL_IDX)
					break;
			}

			w << uint8_t(nvp);
			for (uint32_t j = 0; j < nvp; ++j)
				w << uint16_t(p[j]);
		}
	}

	stream->close();
	stream = nullptr;

	if (!outputInstance->commit())
	{
		log::error << L"NavMesh pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	dtFree(navData);
	navData = nullptr;

	// Save pmesh for debugging; only in editor.
	if (m_editor)
	{
		Ref< model::Model > pmeshModel = new model::Model();

		for (int32_t i = 0; i < pmesh->nverts; ++i)
		{
			pmeshModel->addPosition(Vector4(
				pmesh->bmin[0] + pmesh->verts[i * 3 + 0] * pmesh->cs,
				pmesh->bmin[1] + pmesh->verts[i * 3 + 1] * pmesh->ch,
				pmesh->bmin[2] + pmesh->verts[i * 3 + 2] * pmesh->cs,
				1.0f
			));
			pmeshModel->addVertex(model::Vertex(i));
		}

		for (int32_t i = 0; i < pmesh->npolys; ++i)
		{
			model::Polygon polygon;

			const uint16_t* p = &pmesh->polys[i * pmesh->nvp * 2];
			for (int32_t j = 0; j < pmesh->nvp; ++j)
			{
				if (p[j] == RC_MESH_NULL_IDX)
					break;

				polygon.addVertex(p[j]);
			}

			polygon.flipWinding();

			pmeshModel->addPolygon(polygon);
		}

		model::Triangulate().apply(*pmeshModel);

		model::ModelFormat::writeAny(L"data/Temp/NavMesh_nav.obj", pmeshModel);
	}

	rcFreePolyMeshDetail(dmesh);
	dmesh = nullptr;

	rcFreePolyMesh(pmesh);
	pmesh = nullptr;

	return true;
}

	}
}
