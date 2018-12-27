/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_AutoLodMeshEntity_H
#define traktor_mesh_AutoLodMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class AutoLodMesh;

class T_DLLCLASS AutoLodMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	AutoLodMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< AutoLodMesh >& mesh);
	
	virtual Aabb3 getBoundingBox() const override final;

	virtual bool supportTechnique(render::handle_t technique) const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) override final;

private:
	resource::Proxy< AutoLodMesh > m_mesh;
	float m_lodDistance;
};

	}
}

#endif	// traktor_mesh_AutoLodMeshEntity_H
