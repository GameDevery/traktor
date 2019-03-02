#pragma once

#include "Core/Math/Matrix44.h"
#include "Mesh/AbstractMeshEntityData.h"
#include "Resource/Id.h"

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

class IMesh;

class T_DLLCLASS MeshEntityData : public AbstractMeshEntityData
{
	T_RTTI_CLASS;

public:
	MeshEntityData();

	void setMesh(const resource::Id< IMesh >& mesh);

	const resource::Id< IMesh >& getMesh() const;

	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< IMesh > m_mesh;
	bool m_screenSpaceCulling;
};

	}
}

