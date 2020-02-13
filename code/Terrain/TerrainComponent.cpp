#include <algorithm>
#include <limits>
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/ITerrainLayer.h"
#include "Terrain/ITerrainLayerData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const int32_t c_patchLodSteps = 3;
const int32_t c_surfaceLodSteps = 3;

const Vector4 c_lodColor[] =
{
	Vector4(1.0f, 0.0f, 0.0f, 0.0f),
	Vector4(0.0f, 1.0f, 0.0f, 0.0f),
	Vector4(0.0f, 0.0f, 1.0f, 0.0f),
	Vector4(1.0f, 1.0f, 0.0f, 0.0f)
};

struct CullPatch
{
	float error[4];
	float distance;
	float area;
	uint32_t patchId;
	Vector4 patchOrigin;
};

typedef std::pair< float, const TerrainComponent::Patch* > cull_patch_t;

struct PatchFrontToBackPredicate
{
	bool operator () (const CullPatch& lh, const CullPatch& rh) const
	{
		return lh.distance < rh.distance;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainComponent", TerrainComponent, world::IEntityComponent)

TerrainComponent::TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_cacheSize(0)
,	m_visualizeMode(VmDefault)
,	m_handleSurface(render::getParameterHandle(L"Surface"))
,	m_handleSurfaceOffset(render::getParameterHandle(L"SurfaceOffset"))
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleColorMap(render::getParameterHandle(L"ColorMap"))
,	m_handleSplatMap(render::getParameterHandle(L"SplatMap"))
,	m_handleCutMap(render::getParameterHandle(L"CutMap"))
,	m_handleMaterialMap(render::getParameterHandle(L"MaterialMap"))
,	m_handleNormals(render::getParameterHandle(L"Normals"))
,	m_handleEye(render::getParameterHandle(L"Eye"))
,	m_handleWorldOrigin(render::getParameterHandle(L"WorldOrigin"))
,	m_handleWorldExtent(render::getParameterHandle(L"WorldExtent"))
,	m_handlePatchOrigin(render::getParameterHandle(L"PatchOrigin"))
,	m_handlePatchExtent(render::getParameterHandle(L"PatchExtent"))
,	m_handleDetailDistance(render::getParameterHandle(L"DetailDistance"))
,	m_handleDebugPatchColor(render::getParameterHandle(L"DebugPatchColor"))
,	m_handleDebugMap(render::getParameterHandle(L"DebugMap"))
,	m_handleCutEnable(render::getParameterHandle(L"CutEnable"))
,	m_handleColorEnable(render::getParameterHandle(L"ColorEnable"))
{
}

bool TerrainComponent::create(const TerrainComponentData& data)
{
	if (!m_resourceManager->bind(data.getTerrain(), m_terrain))
		return false;

	m_heightfield = m_terrain->getHeightfield();

	m_patchLodDistance = data.getPatchLodDistance();
	m_patchLodBias = data.getPatchLodBias();
	m_patchLodExponent = data.getPatchLodExponent();
	m_surfaceLodDistance = data.getSurfaceLodDistance();
	m_surfaceLodBias = data.getSurfaceLodBias();
	m_surfaceLodExponent = data.getSurfaceLodExponent();

	if (!createPatches())
		return false;

	for (const auto layerData : data.getLayers())
	{
		Ref< ITerrainLayer > layer = layerData->createLayerInstance(m_resourceManager, m_renderSystem, *this);
		if (layer)
			m_layers.push_back(layer);
	}

	return true;
}

void TerrainComponent::build(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	float detailDistance,
	uint32_t cacheSize
)
{
	if (
		m_terrain.changed() ||
		m_heightfield.changed()
	)
	{
		m_heightfield.consume();
		m_terrain.consume();

		if (!createPatches())
			return;
	}

	if (!m_surfaceCache || cacheSize != m_cacheSize)
	{
		safeDestroy(m_surfaceCache);
		m_surfaceCache = new TerrainSurfaceCache();
		if (!m_surfaceCache->create(m_resourceManager, m_renderSystem, cacheSize))
			return;

		m_cacheSize = cacheSize;
	}

	render::Shader* coarseShader = m_terrain->getTerrainCoarseShader();
	render::Shader* detailShader = m_terrain->getTerrainDetailShader();

	worldRenderPass.setShaderTechnique(coarseShader);
	worldRenderPass.setShaderCombination(coarseShader);

	worldRenderPass.setShaderTechnique(detailShader);
	worldRenderPass.setShaderCombination(detailShader);

	coarseShader->setCombination(m_handleCutEnable, m_terrain->getCutMap());
	detailShader->setCombination(m_handleCutEnable, m_terrain->getCutMap());

	coarseShader->setCombination(m_handleColorEnable, m_terrain->getColorMap());
	detailShader->setCombination(m_handleColorEnable, m_terrain->getColorMap());

	render::IProgram* coarseProgram = coarseShader->getCurrentProgram();
	render::IProgram* detailProgram = detailShader->getCurrentProgram();

	if (!coarseProgram || !detailProgram)
		return;

	// Update cache only once per frame and when rendering from camera.
	bool updateCache = bool((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0);

	const Vector4& worldExtent = m_heightfield->getWorldExtent();

	Matrix44 viewInv = worldRenderView.getView().inverse();
	Matrix44 viewProj = worldRenderView.getProjection() * worldRenderView.getView();
	Vector4 eyePosition = worldRenderView.getEyePosition();
	Vector4 eyeDirection = worldRenderView.getEyeDirection();

	// Cull patches.
	static AlignedVector< CullPatch > visiblePatches;
	visiblePatches.resize(0);

	Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);
	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();
	Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	// Calculate world frustum.
	Frustum viewCullFrustum = worldRenderView.getCullFrustum();
	Frustum worldCullFrustum = viewCullFrustum;
	for (uint32_t i = 0; i < worldCullFrustum.planes.size(); ++i)
		worldCullFrustum.planes[i] = viewInv * worldCullFrustum.planes[i];

	// Cull patches to world frustum.
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		Vector4 patchOrigin = patchTopLeft;
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			uint32_t patchId = px + pz * m_patchCount;

			const Patch& patch = m_patches[patchId];
			Vector4 patchCenterWorld = (patchOrigin + patchDeltaHalf) * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, (patch.minHeight + patch.maxHeight) * 0.5f, 0.0f, 1.0f);

			Aabb3 patchAabb(
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(-patchDeltaHalf.x(), patch.minHeight - FUZZY_EPSILON, -patchDeltaHalf.z(), 0.0f),
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4( patchDeltaHalf.x(), patch.maxHeight + FUZZY_EPSILON,  patchDeltaHalf.z(), 0.0f)
			);

			if (worldCullFrustum.inside(patchAabb) != Frustum::IrOutside)
			{
				Scalar lodDistance = (patchCenterWorld - eyePosition).xyz0().length();
				Vector4 patchCenterWorld_x0zw = patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f);
				Vector4 eyePosition_0y00 = Vector4(0.0f, eyePosition.y(), 0.0f, 0.0f);

				CullPatch cp;

				// Calculate screen error for each lod.
				for (int i = 0; i < LodCount; ++i)
				{
					Vector4 Pworld[2] =
					{
						patchCenterWorld_x0zw + eyePosition_0y00,
						patchCenterWorld_x0zw + eyePosition_0y00 + Vector4(0.0f, patch.error[i], 0.0f, 0.0f)
					};

					Vector4 Pview[2] =
					{
						worldRenderView.getView() * Pworld[0],
						worldRenderView.getView() * Pworld[1]
					};

					if (Pview[0].z() < viewCullFrustum.getNearZ())
						Pview[0].set(2, viewCullFrustum.getNearZ());
					if (Pview[1].z() < viewCullFrustum.getNearZ())
						Pview[1].set(2, viewCullFrustum.getNearZ());

					Vector4 Pclip[] =
					{
						worldRenderView.getProjection() * Pview[0].xyz1(),
						worldRenderView.getProjection() * Pview[1].xyz1()
					};

					T_ASSERT(Pclip[0].w() > 0.0f);
					T_ASSERT(Pclip[1].w() > 0.0f);

					Pclip[0] /= Pclip[0].w();
					Pclip[1] /= Pclip[1].w();

					Vector4 d = Pclip[1] - Pclip[0];

					float dx = d.x();
					float dy = d.y();

					cp.error[i] = std::sqrt(dx * dx + dy * dy) * 100.0f;
				}

				// Project patch bounding box extents onto view plane and calculate screen area.
				Vector4 extents[8];
				patchAabb.getExtents(extents);

				Vector4 mn(
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max()
				);
				Vector4 mx(
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max()
				);

				bool clipped = false;

				for (int32_t i = 0; i < sizeof_array(extents); ++i)
				{
					Vector4 p = viewProj * extents[i];
					if (p.w() <= 0.0f)
					{
						clipped = true;
						break;
					}

					// Homogeneous divide.
					p /= p.w();

					// Track screen space extents.
					mn = min(mn, p);
					mx = max(mx, p);
				}

				Vector4 e = mx - mn;

				cp.distance = lodDistance;
				cp.area = !clipped ? e.x() * e.y() : Scalar(1000.0f);
				cp.patchId = patchId;
				cp.patchOrigin = patchOrigin;

				visiblePatches.push_back(cp);
			}
			else if (updateCache)
			{
				m_patches[patchId].lastPatchLod = c_patchLodSteps;
				m_patches[patchId].lastSurfaceLod = c_surfaceLodSteps;

				m_surfaceCache->flush(patchId);
			}

			patchOrigin += patchDeltaX;
		}
		patchTopLeft += patchDeltaZ;
	}

	// Sort patches front to back to maximize best use of surface cache and rendering.
	std::sort(visiblePatches.begin(), visiblePatches.end(), PatchFrontToBackPredicate());

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	static AlignedVector< const CullPatch* > patchLodInstances[LodCount];
	for (uint32_t i = 0; i < LodCount; ++i)
		patchLodInstances[i].resize(0);
#endif

	// Update all patch surfaces.
	if (updateCache)
	{
		m_surfaceCache->begin(
			worldContext.getRenderContext(),
			m_terrain,
			-worldExtent * Scalar(0.5f),
			worldExtent
		);

		for (const auto& visiblePatch : visiblePatches)
		{
			Patch& patch = m_patches[visiblePatch.patchId];
			const Vector4& patchOrigin = visiblePatch.patchOrigin;

			// Calculate which surface lod to use based one distance to patch center.
			float distance = max(visiblePatch.distance - detailDistance, 0.0f);
			float surfaceLodDistance = std::pow(clamp(distance / m_surfaceLodDistance + m_surfaceLodBias, 0.0f, 1.0f), m_surfaceLodExponent);
			float surfaceLodF = surfaceLodDistance * c_surfaceLodSteps;
			int32_t surfaceLod = int32_t(surfaceLodF + 0.5f);

			const float c_lodHysteresisThreshold = 0.5f;
			if (surfaceLod != patch.lastSurfaceLod)
			{
				if (std::abs(surfaceLodF - patch.lastSurfaceLod) < c_lodHysteresisThreshold)
					surfaceLod = patch.lastSurfaceLod;
			}

			// Find patch lod based on screen space error.
			int32_t patchLod = 0;
			for (int32_t j = 3; j > 0; --j)
			{
				if (visiblePatch.error[j] <= 1.0f)
				{
					patchLod = j;
					break;
				}
			}

			patch.lastPatchLod = patchLod;
			patch.lastSurfaceLod = surfaceLod;

			// Update surface cache.
			m_surfaceCache->get(
				worldContext.getRenderContext(),
				m_terrain,
				-worldExtent * Scalar(0.5f),
				worldExtent,
				patchOrigin,
				patchExtent,
				patch.lastSurfaceLod,
				visiblePatch.patchId,
				// Out
				patch.surfaceOffset
			);

			// Queue patch instance.
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			patchLodInstances[patchLod].push_back(&visiblePatch);
#endif
		}
	}
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	else
	{
		for (const auto& visiblePatch : visiblePatches)
		{
			Patch& patch = m_patches[visiblePatch.patchId];
			patchLodInstances[patch.lastPatchLod].push_back(&visiblePatch);
		}
	}
#endif

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)

	for (const auto& visiblePatch : visiblePatches)
	{
		Patch& patch = m_patches[visiblePatch.patchId];

		auto rb = worldContext.getRenderContext()->alloc< render::SimpleRenderBlock >("Terrain patch");

		rb->distance = visiblePatch.distance;
		rb->program = (patch.lastSurfaceLod == 0) ? detailProgram : coarseProgram;
		rb->programParams = worldContext.getRenderContext()->alloc< render::ProgramParameters >();
		rb->indexBuffer = m_indexBuffer;
		rb->vertexBuffer = m_vertexBuffer;
		rb->primitives = m_primitives[patch.lastPatchLod];

		rb->programParams->beginParameters(worldContext.getRenderContext());
		worldRenderPass.setProgramParameters(rb->programParams, render::RpOpaque);

		rb->programParams->setTextureParameter(m_handleHeightfield, m_terrain->getHeightMap());
		rb->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		rb->programParams->setTextureParameter(m_handleColorMap, m_terrain->getColorMap());
		rb->programParams->setTextureParameter(m_handleNormals, m_terrain->getNormalMap());
		rb->programParams->setTextureParameter(m_handleSplatMap, m_terrain->getSplatMap());
		rb->programParams->setTextureParameter(m_handleCutMap, m_terrain->getCutMap());
		rb->programParams->setTextureParameter(m_handleMaterialMap, m_terrain->getMaterialMap());
		rb->programParams->setVectorParameter(m_handleEye, eyePosition);
		rb->programParams->setVectorParameter(m_handleWorldOrigin, -worldExtent * Scalar(0.5f));
		rb->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		rb->programParams->setVectorParameter(m_handlePatchExtent, patchExtent);
		rb->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		rb->programParams->setVectorParameter(m_handlePatchOrigin, visiblePatch.patchOrigin);
		rb->programParams->setFloatParameter(m_handleDetailDistance, detailDistance);

		if (m_visualizeMode == VmSurfaceLod)
			rb->programParams->setVectorParameter(m_handleDebugPatchColor, c_lodColor[patch.lastSurfaceLod]);
		else if (m_visualizeMode == VmPatchLod)
			rb->programParams->setVectorParameter(m_handleDebugPatchColor, c_lodColor[patch.lastPatchLod]);

		rb->programParams->endParameters(worldContext.getRenderContext());

		worldContext.getRenderContext()->draw(render::RpOpaque, rb);
	}

#else

	render::RenderContext* renderContext = worldContext.getRenderContext();

	// Setup shared shader parameters.
	for (int32_t i = 0; i < 2; ++i)
	{
		auto rb = renderContext->alloc< render::NullRenderBlock >("Terrain patch setup");

		rb->program = (i == 0) ? coarseProgram : detailProgram;
		rb->programParams = renderContext->alloc< render::ProgramParameters >();

		rb->programParams->beginParameters(renderContext);

		rb->programParams->setTextureParameter(m_handleHeightfield, m_terrain->getHeightMap());
		rb->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		rb->programParams->setTextureParameter(m_handleColorMap, m_terrain->getColorMap());
		rb->programParams->setTextureParameter(m_handleNormals, m_terrain->getNormalMap());
		rb->programParams->setTextureParameter(m_handleSplatMap, m_terrain->getSplatMap());
		rb->programParams->setTextureParameter(m_handleCutMap, m_terrain->getCutMap());
		rb->programParams->setTextureParameter(m_handleMaterialMap, m_terrain->getMaterialMap());
		rb->programParams->setVectorParameter(m_handleEye, eyePosition);
		rb->programParams->setVectorParameter(m_handleWorldOrigin, -worldExtent * Scalar(0.5f));
		rb->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		rb->programParams->setFloatParameter(m_handleDetailDistance, detailDistance);

		if (m_visualizeMode == VmColorMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getColorMap());
		else if (m_visualizeMode == VmNormalMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getNormalMap());
		else if (m_visualizeMode == VmHeightMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getHeightMap());
		else if (m_visualizeMode == VmSplatMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getSplatMap());
		else if (m_visualizeMode == VmCutMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getCutMap());
		else if (m_visualizeMode == VmMaterialMap)
			rb->programParams->setTextureParameter(m_handleDebugMap, m_terrain->getMaterialMap());

		worldRenderPass.setProgramParameters(rb->programParams);

		rb->programParams->endParameters(renderContext);

		renderContext->enqueue(rb);
	}

	// Render each visible patch.
	for (const auto& visiblePatch : visiblePatches)
	{
		const Patch& patch = m_patches[visiblePatch.patchId];
		const Vector4& patchOrigin = visiblePatch.patchOrigin;

		auto rb = renderContext->alloc< render::SimpleRenderBlock >("Terrain patch");

		rb->distance = visiblePatch.distance;
		rb->program = (patch.lastSurfaceLod == 0) ? detailProgram : coarseProgram;
		rb->programParams = renderContext->alloc< render::ProgramParameters >();
		rb->indexBuffer = m_indexBuffer;
		rb->vertexBuffer = patch.vertexBuffer;
		rb->primitives = m_primitives[patch.lastPatchLod];

		rb->programParams->beginParameters(renderContext);

		rb->programParams->setVectorParameter(m_handlePatchOrigin, patchOrigin);
		rb->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		rb->programParams->setVectorParameter(m_handlePatchExtent, patchExtent);

		if (m_visualizeMode == VmSurfaceLod)
			rb->programParams->setVectorParameter(m_handleDebugPatchColor, c_lodColor[patch.lastSurfaceLod]);
		else if (m_visualizeMode == VmPatchLod)
			rb->programParams->setVectorParameter(m_handleDebugPatchColor, c_lodColor[patch.lastPatchLod]);

		rb->programParams->endParameters(renderContext);

		renderContext->draw(render::RpOpaque, rb);
	}

#endif
}

void TerrainComponent::buildLayers(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	for (const auto layer : m_layers)
		layer->build(
			*this,
			worldContext,
			worldRenderView,
			worldRenderPass
		);
}

void TerrainComponent::setVisualizeMode(VisualizeMode visualizeMode)
{
	m_visualizeMode = visualizeMode;

	render::Shader* coarseShader = m_terrain->getTerrainCoarseShader();
	render::Shader* detailShader = m_terrain->getTerrainDetailShader();

	if (coarseShader)
	{
		coarseShader->setCombination(L"VisualizeLods", false);
		coarseShader->setCombination(L"VisualizeMap", false);
	}
	if (detailShader)
	{
		detailShader->setCombination(L"VisualizeLods", false);
		detailShader->setCombination(L"VisualizeMap", false);
	}

	if (m_visualizeMode >= VmSurfaceLod && m_visualizeMode <= VmPatchLod)
	{
		if (coarseShader)
			coarseShader->setCombination(L"VisualizeLods", true);
		if (detailShader)
			detailShader->setCombination(L"VisualizeLods", true);
	}
	else if (m_visualizeMode >= VmColorMap && m_visualizeMode <= VmMaterialMap)
	{
		if (coarseShader)
			coarseShader->setCombination(L"VisualizeMap", true);
		if (detailShader)
			detailShader->setCombination(L"VisualizeMap", true);
	}
}

void TerrainComponent::destroy()
{
}

void TerrainComponent::setOwner(world::Entity* owner)
{
}

void TerrainComponent::setTransform(const Transform& transform)
{
}

Aabb3 TerrainComponent::getBoundingBox() const
{
	const Vector4& worldExtent = m_heightfield->getWorldExtent();
	return Aabb3(-worldExtent, worldExtent);
}

void TerrainComponent::update(const world::UpdateParams& update)
{
	for (const auto layer : m_layers)
		layer->update(update);
}

bool TerrainComponent::updatePatches(const uint32_t* region)
{
	uint32_t patchDim = m_terrain->getPatchDim();
	uint32_t heightfieldSize = m_heightfield->getSize();

	uint32_t mnx = region ? max< uint32_t >(region[0], 0) : 0;
	uint32_t mnz = region ? max< uint32_t >(region[1], 0) : 0;
	uint32_t mxx = region ? min< uint32_t >(region[2] + 1, m_patchCount) : m_patchCount;
	uint32_t mxz = region ? min< uint32_t >(region[3] + 1, m_patchCount) : m_patchCount;

	for (uint32_t pz = mnz; pz < mxz; ++pz)
	{
		for (uint32_t px = mnx; px < mxx; ++px)
		{
			uint32_t patchId = px + pz * m_patchCount;

			int32_t pminX = (heightfieldSize * px) / m_patchCount;
			int32_t pminZ = (heightfieldSize * pz) / m_patchCount;
			int32_t pmaxX = (heightfieldSize * (px + 1)) / m_patchCount;
			int32_t pmaxZ = (heightfieldSize * (pz + 1)) / m_patchCount;

			const Terrain::Patch& patchData = m_terrain->getPatches()[patchId];
			Patch& patch = m_patches[patchId];

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			float* vertex = static_cast< float* >(patch.vertexBuffer->lock());
			T_ASSERT(vertex);

			for (uint32_t z = 0; z < patchDim; ++z)
			{
				for (uint32_t x = 0; x < patchDim; ++x)
				{
					float fx = float(x) / (patchDim - 1);
					float fz = float(z) / (patchDim - 1);

					int32_t ix = int32_t(fx * (pmaxX - pminX)) + pminX;
					int32_t iz = int32_t(fz * (pmaxZ - pminZ)) + pminZ;

					float height = m_heightfield->getGridHeightNearest(ix, iz);

					*vertex++ = fx;
					*vertex++ = height;
					*vertex++ = fz;
				}
			}

			patch.vertexBuffer->unlock();
#endif

			patch.minHeight = patchData.height[0];
			patch.maxHeight = patchData.height[1];

			patch.error[0] = 0.0f;
			patch.error[1] = patchData.error[0];
			patch.error[2] = patchData.error[1];
			patch.error[3] = patchData.error[2];

			if (m_surfaceCache)
				m_surfaceCache->flush(patchId);
		}
	}

	for (const auto layer : m_layers)
		layer->updatePatches(*this);

	return true;
}

bool TerrainComponent::createPatches()
{
	m_patches.clear();
	m_patchCount = 0;
	safeDestroy(m_indexBuffer);
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	safeDestroy(m_vertexBuffer);
#endif

	uint32_t heightfieldSize = m_heightfield->getSize();
	T_ASSERT(heightfieldSize > 0);

	uint32_t patchDim = m_terrain->getPatchDim();
	uint32_t detailSkip = m_terrain->getDetailSkip();

	uint32_t patchVertexCount = patchDim * patchDim;
	m_patchCount = heightfieldSize / (patchDim * detailSkip);

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));
	uint32_t vertexSize = render::getVertexSize(vertexElements);

	m_vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		patchVertexCount * vertexSize,
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t z = 0; z < patchDim; ++z)
	{
		for (uint32_t x = 0; x < patchDim; ++x)
		{
			*vertex++ = float(x) / (patchDim - 1);
			*vertex++ = float(z) / (patchDim - 1);
		}
	}

	m_vertexBuffer->unlock();
#else
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, 0));
	uint32_t vertexSize = render::getVertexSize(vertexElements);
#endif

	m_patches.reserve(m_patchCount * m_patchCount);
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
				vertexElements,
				patchVertexCount * vertexSize,
				false
			);
			if (!vertexBuffer)
				return false;

			TerrainComponent::Patch patch = { 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f }, vertexBuffer, c_patchLodSteps, c_surfaceLodSteps };
			m_patches.push_back(patch);
#else
			TerrainComponent::Patch patch = { 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f }, c_patchLodSteps, c_surfaceLodSteps };
			m_patches.push_back(patch);
#endif
		}
	}

	updatePatches(nullptr);

	AlignedVector< uint32_t > indices;
	for (uint32_t lod = 0; lod < LodCount; ++lod)
	{
		uint32_t indexOffset = uint32_t(indices.size());
		uint32_t lodSkip = 1 << lod;

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)

		for (uint32_t y = 0; y < patchDim - 1; y += lodSkip)
		{
			uint32_t offset = y * patchDim;
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == patchDim - 1 - lodSkip || y == patchDim - 1 - lodSkip))
				{
					int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * patchDim + patchDim);
							indices.push_back(offset + i * patchDim);
						}
					}
					else if (x == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * patchDim + lodSkip);
							indices.push_back(x + offset + i * patchDim + lodSkip + patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);
				}
			}
		}

		uint32_t indexEndOffset = uint32_t(indices.size());
		T_ASSERT((indexEndOffset - indexOffset) % 3 == 0);

		uint32_t minIndex = *std::min_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);
		uint32_t maxIndex = *std::max_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);

		T_ASSERT(minIndex < patchVertexCount);
		T_ASSERT(maxIndex < patchVertexCount);

		m_primitives[lod].setIndexed(
			render::PtTriangles,
			indexOffset,
			(indexEndOffset - indexOffset) / 3,
			minIndex,
			maxIndex
		);
#else

		for (uint32_t y = 0; y < patchDim - 1; y += lodSkip)
		{
			uint32_t offset = y * patchDim;
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == patchDim - 1 - lodSkip || y == patchDim - 1 - lodSkip))
				{
					int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * patchDim + patchDim);
							indices.push_back(offset + i * patchDim);
						}
					}
					else if (x == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * patchDim + lodSkip);
							indices.push_back(x + offset + i * patchDim + lodSkip + patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);
				}
			}
		}

		uint32_t indexEndOffset = uint32_t(indices.size());

		uint32_t minIndex = *std::min_element(indices.c_ptr() + indexOffset, indices.c_ptr() + indexEndOffset);
		uint32_t maxIndex = *std::max_element(indices.c_ptr() + indexOffset, indices.c_ptr() + indexEndOffset);

		T_FATAL_ASSERT (minIndex < patchVertexCount);
		T_FATAL_ASSERT (maxIndex < patchVertexCount);
		T_FATAL_ASSERT ((indexEndOffset - indexOffset) % 3 == 0);

		m_primitives[lod].setIndexed(
			render::PtTriangles,
			indexOffset,
			(indexEndOffset - indexOffset) / 3,
			minIndex,
			maxIndex
		);

#endif
	}

	m_indexBuffer = m_renderSystem->createIndexBuffer(
		render::ItUInt32,
		uint32_t(indices.size() * sizeof(uint32_t)),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();
	return true;
}

	}
}
