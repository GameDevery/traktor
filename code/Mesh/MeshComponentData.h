#pragma once

#include "Core/Ref.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;

	}

	namespace mesh
	{

class IMesh;
class MeshComponent;

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	MeshComponentData();

	explicit MeshComponentData(const resource::Id< IMesh >& mesh);

	Ref< MeshComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	void setMesh(const resource::Id< IMesh >& mesh) { m_mesh = mesh; }

	const resource::Id< IMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< IMesh > m_mesh;
	bool m_screenSpaceCulling = false;
};

	}
}

