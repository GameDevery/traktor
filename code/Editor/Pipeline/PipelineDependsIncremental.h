#pragma once

#include <map>
#include "Core/Timer/Timer.h"
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IPipelineDb;
class PipelineDependencySet;
class IPipelineInstanceCache;
class PipelineFactory;

/*! Incremental pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsIncremental : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	PipelineDependsIncremental(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		PipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb,
		IPipelineInstanceCache* instanceCache,
		uint32_t recursionDepth = ~0U
	);

	virtual void addDependency(
		const ISerializable* sourceAsset
	) override final;

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) override final;

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) override final;

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) override final;

	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	) override final;

	virtual void addDependency(
		const TypeInfo& sourceAssetType
	) override final;

	virtual bool waitUntilFinished() override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final;

private:
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< PipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	Ref< PipelineDependency > m_currentDependency;
	bool m_result;

#if defined(_DEBUG)
	Timer m_timer;
	std::vector< double > m_buildDepTimeStack;
	std::map< const TypeInfo*, std::pair< int32_t, double > > m_buildDepTimes;
#endif

	void addUniqueDependency(
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const IPipeline* pipeline,
		const db::Instance* sourceInstance
	);
};

	}
}

