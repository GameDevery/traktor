#pragma once

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS PointSetPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::IPipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const override final;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const override final;

private:
	std::wstring m_assetPath;
};

	}
}

