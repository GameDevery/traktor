/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_EntityRenderer_H
#define traktor_terrain_EntityRenderer_H

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

/*! \brief Terrain entity renderer.
 * \ingroup Terrain
 */
class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EntityRenderer(float terrainDetailDistance, uint32_t terrainCacheSize, bool terrainLayersEnable, bool oceanReflectionEnable);

	void setTerrainDetailDistance(float terrainDetailDistance);

	void setTerrainCacheSize(uint32_t terrainCacheSize);

	void setTerrainLayersEnable(bool terrainLayersEnable);

	void setOceanDynamicReflectionEnable(bool oceanReflectionEnable);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	float m_terrainDetailDistance;
	uint32_t m_terrainCacheSize;
	bool m_terrainLayersEnable;
	bool m_oceanReflectionEnable;
};

	}
}

#endif	// traktor_terrain_EntityRenderer_H
