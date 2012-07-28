#include <limits>
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/UndergrowthEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntity >());
	typeSet.insert(&type_of< OceanEntity >());
	typeSet.insert(&type_of< RiverEntity >());
	typeSet.insert(&type_of< UndergrowthEntity >());
	return typeSet;
}

void EntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	if (TerrainEntity* terrainEntity = dynamic_type_cast< TerrainEntity* >(entity))
		terrainEntity->render(worldContext, worldRenderView, worldRenderPass);
	else if (OceanEntity* oceanEntity = dynamic_type_cast< OceanEntity* >(entity))
		oceanEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
	else if (RiverEntity* riverEntity = dynamic_type_cast< RiverEntity* >(entity))
		riverEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
	else if (UndergrowthEntity* undergrowthEntity = dynamic_type_cast< UndergrowthEntity* >(entity))
		undergrowthEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
}

void EntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
