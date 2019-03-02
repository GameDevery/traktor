#pragma once

#include "Core/Thread/Semaphore.h"
#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS HeightfieldTexturePipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	HeightfieldTexturePipeline();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

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

private:
	enum HeightFormatType
	{
		HfFloat16 = 0,
		HfFloat32 = 1,
		HfARGBEncoded = 2
	};

	std::wstring m_assetPath;
	HeightFormatType m_heightFormat;
	bool m_compressNormals;
	mutable Semaphore m_lock;
};

	}
}

