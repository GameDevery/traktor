#include "Core/Serialization/DeepClone.h"
#include "Animation/Editor/AnimatedMeshComponentPipeline.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Editor/IPipelineDepends.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimatedMeshComponentPipeline", 0, AnimatedMeshComponentPipeline, editor::IPipeline)

bool AnimatedMeshComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void AnimatedMeshComponentPipeline::destroy()
{
}

TypeInfoSet AnimatedMeshComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponentData >();
}

bool AnimatedMeshComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const AnimatedMeshComponentData* meshComponentData = dynamic_type_cast< const AnimatedMeshComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(meshComponentData->getSkeleton(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(meshComponentData->getPoseControllerData());

		for (auto binding : meshComponentData->getBindings())
			pipelineDepends->addDependency(binding.entityData);
	}
	return true;
}

bool AnimatedMeshComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > AnimatedMeshComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
