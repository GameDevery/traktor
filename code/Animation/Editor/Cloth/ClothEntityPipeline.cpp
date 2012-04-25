#include "Animation/Cloth/ClothEntityData.h"
#include "Animation/Editor/Cloth/ClothEntityPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothEntityPipeline", 0, ClothEntityPipeline, world::EntityPipeline)

TypeInfoSet ClothEntityPipeline::getAssetTypes() const
{
	TypeInfoSet TypeInfoSet;
	TypeInfoSet.insert(&type_of< ClothEntityData >());
	return TypeInfoSet;
}

bool ClothEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	if (const ClothEntityData* clothEntityData = dynamic_type_cast< const ClothEntityData* >(sourceAsset))
		pipelineDepends->addDependency(clothEntityData->getShader(), editor::PdfBuild);
	return true;
}

	}
}
