#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
class WorldBuildContext;
class WorldGatherContext;
class WorldRenderView;
class WorldSetupContext;

/*! Entity renderer.
 * \ingroup World
 *
 * Each renderable type should have
 * a matching EntityRenderer.
 */
class T_DLLCLASS IEntityRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const = 0;

	/*! Gather pass. 
	 *
	 * Called once per frame to gather active renderables.
	 *
	 * \param context World context.
	 * \param renderable Renderable instance.
	 */
	virtual void gather(
		const WorldGatherContext& context,
		Object* renderable
	) = 0;

	/*! Setup pass. */
	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		Object* renderable
	) = 0;

	/*! Setup pass. */
	virtual void setup(
		const WorldSetupContext& context
	) = 0;

	/*! Build pass. */
	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) = 0;

	/*! Build pass.
	 *
	 * Flush whatever queues that the entity
	 * renderer might have used.
	 */
	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) = 0;
};

	}
}

