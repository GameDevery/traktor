#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/Assets.h"
#include "Editor/AssetsPipeline.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AssetsPipeline", 1, AssetsPipeline, IPipeline)

bool AssetsPipeline::create(const IPipelineSettings* settings)
{
	m_editorDeploy = settings->getProperty< bool >(L"Pipeline.EditorDeploy", false);
	return true;
}

void AssetsPipeline::destroy()
{
}

TypeInfoSet AssetsPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< Assets >();
}

uint32_t AssetsPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool AssetsPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const Assets* assets = mandatory_non_null_type_cast< const Assets* >(sourceAsset);
	for (const auto& dependency : assets->m_dependencies)
	{
		if (!dependency.editorDeployOnly || m_editorDeploy)
			pipelineDepends->addDependency(dependency.id, editor::PdfBuild);
	}
	return true;
}

bool AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	return true;
}

Ref< ISerializable > AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
