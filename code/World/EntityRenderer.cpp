#include "World/Entity.h"
#include "World/EntityRenderer.h"
#include "World/IEntityComponent.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/WorldSetupContext.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityRenderer", EntityRenderer, IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< Entity >();
}

void EntityRenderer::gather(
	const WorldGatherContext& context,
	Object* renderable
)
{
	const Entity* entity = static_cast< const Entity* >(renderable);
	for (auto component : entity->getComponents())
		context.gather(component);
}

void EntityRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void EntityRenderer::setup(
	const WorldSetupContext& context
)
{
}

void EntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void EntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

	}
}
