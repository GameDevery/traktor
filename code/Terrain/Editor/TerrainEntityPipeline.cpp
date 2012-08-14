#include "Terrain/Editor/TerrainEntityPipeline.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/RiverEntityData.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEntityPipeline", 0, TerrainEntityPipeline, world::EntityPipeline)

TypeInfoSet TerrainEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< RiverEntityData >());
	typeSet.insert(&type_of< UndergrowthEntityData >());
	return typeSet;
}

bool TerrainEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	if (const TerrainEntityData* terrainEntityData = dynamic_type_cast< const TerrainEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(terrainEntityData->getTerrain(), editor::PdfBuild);
	}
	else if (const OceanEntityData* oceanEntityData = dynamic_type_cast< const OceanEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(oceanEntityData->getTerrain(), editor::PdfBuild);
		pipelineDepends->addDependency(oceanEntityData->getShaderWaves(), editor::PdfBuild);
		pipelineDepends->addDependency(oceanEntityData->getShaderComposite(), editor::PdfBuild);
	}
	else if (const RiverEntityData* riverEntityData = dynamic_type_cast< const RiverEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(riverEntityData->getShader(), editor::PdfBuild);
	}
	else if (const UndergrowthEntityData* undergrowthEntityData = dynamic_type_cast< const UndergrowthEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(undergrowthEntityData->getTerrain(), editor::PdfBuild);
		pipelineDepends->addDependency(undergrowthEntityData->getMaterialMask(), editor::PdfBuild);
		pipelineDepends->addDependency(undergrowthEntityData->getShader(), editor::PdfBuild);
	}
	return true;
}

	}
}
