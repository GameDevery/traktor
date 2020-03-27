#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Resource/IResourceManager.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/UndergrowthComponent.h"
#include "Terrain/UndergrowthComponentData.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

#if !defined(__PS3__)
const int32_t c_maxInstanceCount = 180;
#else
const int32_t c_maxInstanceCount = 20;
#endif

#pragma pack(1)
struct Vertex
{
	float position[2];
	half_t texCoord[2];
};
#pragma pack()

const render::Handle s_handleNormals(L"Normals");
const render::Handle s_handleHeightfield(L"Heightfield");
const render::Handle s_handleSurface(L"Surface");
const render::Handle s_handleWorldExtent(L"WorldExtent");
const render::Handle s_handleEye(L"Eye");
const render::Handle s_handleMaxDistance(L"MaxDistance");
const render::Handle s_handleInstances1(L"Instances1");
const render::Handle s_handleInstances2(L"Instances2");

Vertex packVertex(const Vector4& position, float u, float v)
{
	Vertex vtx;
	vtx.position[0] = position.x();
	vtx.position[1] = position.y();
	vtx.texCoord[0] = floatToHalf(u);
	vtx.texCoord[1] = floatToHalf(v);
	return vtx;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthComponent", UndergrowthComponent, TerrainLayerComponent)

UndergrowthComponent::UndergrowthComponent()
:	m_owner(nullptr)
,	m_clusterSize(0.0f)
,	m_plantsCount(0)
{
}

bool UndergrowthComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const UndergrowthComponentData& layerData
)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_shader, m_shader))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtHalf2, offsetof(Vertex, texCoord)));
	T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		4 * sizeof(Vertex),
		false
	);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	Vector4 position(0.0f, 0.0f, 0.0f);
	Vector4 axisX(1.0f, 0.0f, 0.0f);
	Vector4 axisY(0.0f, 1.0f, 0.0f);

	*vertex++ = packVertex(position - axisX - axisY, 0.0f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY, 0.0f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY, 1.0f, 0.0f);
	*vertex++ = packVertex(position + axisX - axisY, 1.0f, 1.0f);

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		3 * 2 * 2 * sizeof(uint16_t),
		false
	);
	if (!m_indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());

	*index++ = 0;
	*index++ = 1;
	*index++ = 2;

	*index++ = 0;
	*index++ = 2;
	*index++ = 3;

	*index++ = 2;
	*index++ = 1;
	*index++ = 0;

	*index++ = 3;
	*index++ = 2;
	*index++ = 0;

	m_indexBuffer->unlock();
	return true;
}

void UndergrowthComponent::destroy()
{
}

void UndergrowthComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void UndergrowthComponent::setTransform(const Transform& transform)
{
}

Aabb3 UndergrowthComponent::getBoundingBox() const
{
	return Aabb3();
}

void UndergrowthComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void UndergrowthComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = bool((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInv = view.inverse();
	Vector4 eye = viewInv.translation();

	// Get plant state for current view.
	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (vs.plants.size() != m_plantsCount * 2)
	{
		vs.plants.resize(m_plantsCount * 2, Vector4::zero());
		vs.distances.resize(m_clusters.size(), 0.0f);
		vs.pvs.assign((uint32_t)m_clusters.size(), false);
		updateClusters = true;
	}

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

		// Only perform "replanting" half of clusters each frame.
		const Scalar clusterSize(m_clusterSize);
		for (uint32_t i = vs.count % 2; i < m_clusters.size(); i += 2)
		{
			const Cluster& cluster = m_clusters[i];

			vs.distances[i] = (cluster.center - eye).length();

			bool visible = vs.pvs[i];
			vs.pvs.set(i, viewFrustum.inside(view * cluster.center, clusterSize) != Frustum::IrOutside);
			if (!vs.pvs[i])
				continue;
			if (vs.pvs[i] && visible)
				continue;

			RandomGeometry random(int32_t(cluster.center.x() * 919.0f + cluster.center.z() * 463.0f));
			for (int32_t j = cluster.from; j < cluster.to; ++j)
			{
				Vector2 ruv = Quasirandom::hammersley(j - cluster.from, cluster.to - cluster.from, random);

				float dx = (ruv.x * 2.0f - 1.0f) * m_clusterSize;
				float dz = (ruv.y * 2.0f - 1.0f) * m_clusterSize;

				float px = cluster.center.x() + dx;
				float pz = cluster.center.z() + dz;

				vs.plants[j * 2 + 0] = Vector4(
					px,
					pz,
					float(cluster.plant),
					0.0f
				);
				vs.plants[j * 2 + 1] = Vector4(
					cluster.plantScale * (random.nextFloat() * 0.5f + 0.5f),
					random.nextFloat(),
					0.0f,
					0.0f
				);
			}
		}

		vs.count++;
	}

	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	Vector4 instanceData1[c_maxInstanceCount];
	Vector4 instanceData2[c_maxInstanceCount];

	for (uint32_t i = 0; i < m_clusters.size(); ++i)
	{
		if (!vs.pvs[i])
			continue;

		const Cluster& cluster = m_clusters[i];

		int32_t count = cluster.to - cluster.from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min(count - j, c_maxInstanceCount);

			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				instanceData1[k] = vs.plants[(j + cluster.from) * 2 + 0];
				instanceData2[k] = vs.plants[(j + cluster.from) * 2 + 1];
			}

			render::IndexedInstancingRenderBlock* renderBlock = renderContext->alloc< render::IndexedInstancingRenderBlock >();

			renderBlock->distance = vs.distances[i];
			renderBlock->program = sp.program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_indexBuffer;
			renderBlock->vertexBuffer = m_vertexBuffer;
			renderBlock->primitive = render::PtTriangles;
			renderBlock->offset = 0;
			renderBlock->count = 2 * 2;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = 3;
			renderBlock->instanceCount = batch;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(renderBlock->programParams);
			renderBlock->programParams->setTextureParameter(s_handleNormals, terrain->getNormalMap());
			renderBlock->programParams->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
			renderBlock->programParams->setTextureParameter(s_handleSurface, terrainComponent->getSurfaceCache(worldRenderView.getIndex())->getBaseTexture());
			renderBlock->programParams->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
			renderBlock->programParams->setVectorParameter(s_handleEye, eye);
			renderBlock->programParams->setFloatParameter(s_handleMaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
			renderBlock->programParams->setVectorArrayParameter(s_handleInstances1, instanceData1, count);
			renderBlock->programParams->setVectorArrayParameter(s_handleInstances2, instanceData2, count);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				sp.priority,
				renderBlock
			);
		}
	}
}

void UndergrowthComponent::updatePatches()
{
	m_clusters.resize(0);
	m_plantsCount = 0;

	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const resource::Proxy< Terrain >& terrain = terrainComponent->getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um(16, 0);
	uint8_t maxMaterialIndex = 0;
	for (const auto& plant : m_layerData.m_plants)
		um[plant.material] = ++maxMaterialIndex;

	int32_t size = heightfield->getSize();
	Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm(16, 0);
			int32_t totalDensity = 0;

			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
					uint8_t index = um[material];
					if (index > 0)
					{
						cm[index - 1]++;
						totalDensity++;
					}
				}
			}

			if (totalDensity <= 0)
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);

			float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				for (const auto& plant : m_layerData.m_plants)
				{
					if (um[plant.material] == i + 1)
					{
						int32_t densityFactor = cm[i];

						int32_t density = (plant.density * densityFactor) / (16 * 16);
						if (density <= 4)
							continue;

						int32_t from = m_plantsCount;
						int32_t to = from + density;

						Cluster c;
						c.center = Vector4(wx, wy, wz, 1.0f);
						c.plant = plant.plant;
						c.plantScale = plant.scale * (0.5f + 0.5f * densityFactor / (16.0f * 16.0f));
						c.from = from;
						c.to = to;
						m_clusters.push_back(c);

						m_plantsCount = to;
					}
				}
			}
		}
	}
}

	}
}
