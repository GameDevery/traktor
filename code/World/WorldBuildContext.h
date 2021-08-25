#pragma once

#include "Core/Object.h"
#include "World/WorldTypes.h"

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

class RenderContext;

	}

	namespace world
	{

class Entity;
class WorldEntityRenderers;

/*! World build context.
 * \ingroup World
 */
class T_DLLCLASS WorldBuildContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldBuildContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderContext* renderContext);

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	const Entity* getRootEntity() const { return m_rootEntity; }

	render::RenderContext* getRenderContext() const { return m_renderContext; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	const Entity* m_rootEntity;
	render::RenderContext* m_renderContext;
};

	}
}

