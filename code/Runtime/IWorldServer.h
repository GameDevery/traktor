#pragma once

#include "Runtime/IServer.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class IFeedbackManager;

	}

	namespace world
	{

class IEntityBuilder;
class IEntityEventManager;
class IEntityFactory;
class IEntityRenderer;
class IWorldRenderer;
class WorldEntityRenderers;
class WorldRenderSettings;

	}

	namespace runtime
	{

/*! World server.
 * \ingroup Runtime
 *
 * "World.ShadowQuality" - Shadow filter quality.
 */
class T_DLLCLASS IWorldServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual void addEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual const world::IEntityBuilder* getEntityBuilder() = 0;

	virtual world::WorldEntityRenderers* getEntityRenderers() = 0;

	virtual world::IEntityEventManager* getEntityEventManager() = 0;

	virtual spray::IFeedbackManager* getFeedbackManager() = 0;

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings) = 0;
};

	}
}

