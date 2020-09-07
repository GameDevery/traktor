#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.PrecipitationRenderer", PrecipitationRenderer, world::IEntityRenderer)

const TypeInfoSet PrecipitationRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< PrecipitationComponent >();
}

void PrecipitationRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void PrecipitationRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void PrecipitationRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void PrecipitationRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto precipitationComponent = mandatory_non_null_type_cast< PrecipitationComponent* >(renderable);
	precipitationComponent->build(context, worldRenderView, worldRenderPass);
}

void PrecipitationRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
