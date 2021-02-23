#pragma once

#include "Scene/Editor/IEntityReplicator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace terrain
    {

class T_DLLCLASS TerrainEntityReplicator : public scene::IEntityReplicator
{
    T_RTTI_CLASS;

public:
    virtual bool create(const editor::IPipelineSettings* settings) override final;

    virtual TypeInfoSet getSupportedTypes() const override final;

    virtual bool addDependencies(
        editor::IPipelineDepends* pipelineDepends,
        const world::EntityData* entityData,
        const world::IEntityComponentData* componentData
    ) const override final;

    virtual Ref< model::Model > createModel(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
	    const world::EntityData* entityData,
	    const world::IEntityComponentData* componentData
    ) const override final;

    virtual Ref< Object > modifyOutput(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
        const world::EntityData* entityData,
        const world::IEntityComponentData* componentData,
        const model::Model* model,
        const Guid& outputGuid
    ) const override final;
};

    }
}