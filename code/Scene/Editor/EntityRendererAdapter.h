/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_EntityRendererAdapter_H
#define traktor_scene_EntityRendererAdapter_H

#include <functional>
#include <Core/Ref.h>
#include <World/IEntityRenderer.h>

namespace traktor
{
	namespace scene
	{
	
class EntityRendererCache;
	
class EntityRendererAdapter : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer, const std::function< bool(const EntityAdapter*) >& filter);
	
	virtual const TypeInfoSet getRenderableTypes() const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

private:
	Ref< EntityRendererCache > m_cache;
	Ref< world::IEntityRenderer > m_entityRenderer;
	std::function< bool(const EntityAdapter*) > m_filter;
};
	
	}
}

#endif	// traktor_scene_EntityRendererAdapter_H
