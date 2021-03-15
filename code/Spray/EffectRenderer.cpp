#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectRenderer.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/TrailRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectRenderer", EffectRenderer, world::IEntityRenderer)

EffectRenderer::EffectRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance)
:	m_pointRenderer(new PointRenderer(renderSystem, lod1Distance, lod2Distance))
,	m_meshRenderer(new MeshRenderer())
,	m_trailRenderer(new TrailRenderer(renderSystem))
{
}

void EffectRenderer::setLodDistances(float lod1Distance, float lod2Distance)
{
	m_pointRenderer->setLodDistances(lod1Distance, lod2Distance);
}

const TypeInfoSet EffectRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< EffectComponent >();
}

void EffectRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< const world::LightComponent* >& outLights,
	AlignedVector< const world::ProbeComponent* >& outProbes
)
{
}

void EffectRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void EffectRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void EffectRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto effectComponent = mandatory_non_null_type_cast< EffectComponent* >(renderable);

	// Do we need to render anything with this technique?
	if (!effectComponent->haveTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = effectComponent->getWorldBoundingBox();
	if (boundingBox.empty())
		return;

	// Early out of bounding sphere is outside of frustum.
	Vector4 center = worldRenderView.getView() * boundingBox.getCenter().xyz1();
	Scalar radius = boundingBox.getExtent().length();
	if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
		return;

	Vector4 cameraPosition = worldRenderView.getEyePosition();
	Plane cameraPlane(worldRenderView.getEyeDirection(), cameraPosition);

	effectComponent->render(
		worldRenderPass.getTechnique(),
		cameraPosition,
		cameraPlane,
		m_pointRenderer,
		m_meshRenderer,
		m_trailRenderer
	);
}

void EffectRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_pointRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_meshRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_trailRenderer->flush(context.getRenderContext(), worldRenderPass);
}

	}
}
