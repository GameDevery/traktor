#ifndef traktor_mesh_CompositeMeshEntityData_H
#define traktor_mesh_CompositeMeshEntityData_H

#include "Core/RefArray.h"
#include "Mesh/AbstractMeshEntityData.h"

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

class T_DLLCLASS CompositeMeshEntityData : public AbstractMeshEntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(ISerializer& s);
	
	virtual void setTransform(const Transform& transform);

	inline const RefArray< AbstractMeshEntityData >& getEntityData() const { return m_entityData; }

private:
	RefArray< AbstractMeshEntityData > m_entityData;
};

	}
}

#endif	// traktor_mesh_CompositeMeshEntityData_H
