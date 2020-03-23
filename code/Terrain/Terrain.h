#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace render
	{

class ISimpleTexture;
class Shader;

	}

	namespace terrain
	{

/*! Terrain runtime description.
 * \ingroup Terrain
 */
class T_DLLCLASS Terrain : public Object
{
	T_RTTI_CLASS;

public:
	struct Patch
	{
		float height[2];
		float error[3];
	};

	Terrain();

	/*! Get amount of detail to skip.
	 *
	 * \return Level of detail skipped.
	 */
	uint32_t getDetailSkip() const { return m_detailSkip; }

	/*! Get size of a single patch.
	 *
	 * \return Size of single patch.
	 */
	uint32_t getPatchDim() const { return m_patchDim; }

	/*! Get heightfield resource.
	 *
	 * \return Heightfield resource.
	 */
	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	/*! Get terrain color map resource.
	 *
	 * \return Color map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getColorMap() const { return m_colorMap; }

	/*! Get terrain normal map resource.
	 *
	 * \return Normal map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	/*! Get heightmap resource.
	 *
	 * \return Heightmap resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	/*! Get material splat map resource.
	 *
	 * \return Material splat map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getSplatMap() const { return m_splatMap; }

	/*! Get terrain cut map resource.
	 *
	 * \return Cut map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getCutMap() const { return m_cutMap; }

	/*! Get terrain material mask map resource.
	 *
	 * \return Material mask map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getMaterialMap() const { return m_materialMap; }

	/*! Get terrain shader resource.
	 *
	 * \return Terrain shader resource.
	 */
	const resource::Proxy< render::Shader >& getTerrainShader() const { return m_terrainShader; }

	/*! Get terrain surface shader resource.
	 *
	 * \return Terrain surface shader resource.
	 */
	const resource::Proxy< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	/*! Get vector of patch information.
	 *
	 * \return Information about each patch.
	 */
	const AlignedVector< Patch >& getPatches() const { return m_patches; }

	/*! Get vector of patch information.
	 *
	 * \return Information about each patch.
	 */
	AlignedVector< Patch >& editPatches() { return m_patches; }

private:
	friend class TerrainFactory;
	friend class TerrainEditModifier;

	uint32_t m_detailSkip;
	uint32_t m_patchDim;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::ISimpleTexture > m_colorMap;
	resource::Proxy< render::ISimpleTexture > m_normalMap;
	resource::Proxy< render::ISimpleTexture > m_heightMap;
	resource::Proxy< render::ISimpleTexture > m_splatMap;
	resource::Proxy< render::ISimpleTexture > m_cutMap;
	resource::Proxy< render::ISimpleTexture > m_materialMap;
	resource::Proxy< render::Shader > m_terrainShader;
	resource::Proxy< render::Shader > m_surfaceShader;
	AlignedVector< Patch > m_patches;
};

	}
}

