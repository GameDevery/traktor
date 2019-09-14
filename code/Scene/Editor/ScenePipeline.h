#pragma once

#include "Core/RefArray.h"
#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class IScenePipelineOperator;

class T_DLLCLASS ScenePipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	ScenePipeline();

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
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const override final;

private:
	bool m_targetEditor;
	bool m_suppressShadows;
	bool m_suppressLinearLighting;
	bool m_suppressDepthPass;
	bool m_suppressImageProcess;
	int32_t m_shadowMapSizeDenom;
	int32_t m_shadowMapMaxSlices;
	RefArray< IScenePipelineOperator > m_operators;

	const IScenePipelineOperator* findOperator(const TypeInfo& operationType) const;
};

	}
}
