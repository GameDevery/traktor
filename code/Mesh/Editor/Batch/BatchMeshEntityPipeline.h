#ifndef traktor_mesh_BatchMeshEntityPipeline_H
#define traktor_mesh_BatchMeshEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS BatchMeshEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings);

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const;

private:
	std::wstring m_assetPath;
};

	}
}

#endif	// traktor_mesh_BatchMeshEntityPipeline_H
