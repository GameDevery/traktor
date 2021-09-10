#pragma once

#include "World/IWorldRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;

	}

	namespace world
	{

class WorldEntityRenderers;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple world renderer, no support for
 * shadows, lights, postprocessing etc.
 *
 * Techniques used
 * "Simple" - Visual, final, color output.
 */
class T_DLLCLASS WorldRendererSimple : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) override final;

	virtual render::ImageGraphContext* getImageGraphContext() const override final;

private:
	struct Gather
	{
		IEntityRenderer* entityRenderer;
		Object* renderable;
	};

	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::ISimpleTexture > m_depthTexture;
	AlignedVector< Gather > m_gathered;
};

	}
}
