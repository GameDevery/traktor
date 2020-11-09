#include "Editor/IPipelineDepends.h"
#include "Model/Material.h"
#include "Model/Editor/MaterialPipeline.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.MaterialPipeline", 0, MaterialPipeline, editor::IPipeline)

bool MaterialPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void MaterialPipeline::destroy()
{
}

TypeInfoSet MaterialPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< Material >();
}

bool MaterialPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const Material* material = checked_type_cast< const Material*, false >(sourceAsset);
	pipelineDepends->addDependency(material->getDiffuseMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getSpecularMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getRoughnessMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getMetalnessMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getTransparencyMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getEmissiveMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getReflectiveMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getNormalMap().texture, editor::PdfBuild);
	pipelineDepends->addDependency(material->getLightMap().texture, editor::PdfBuild);
	return true;
}

bool MaterialPipeline::buildOutput(
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
	return true;
}

Ref< ISerializable > MaterialPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return nullptr;
}

	}
}
