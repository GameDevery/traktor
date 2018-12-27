/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_BatchMeshEntityData_H
#define traktor_mesh_BatchMeshEntityData_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/EntityData.h"

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

class T_DLLCLASS BatchMeshEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	BatchMeshEntityData();

	virtual void serialize(ISerializer& s) override final;
	
	virtual void setTransform(const Transform& transform) override final;

	const Guid& getOutputGuid() const { return m_outputGuid; }

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

private:
	Guid m_outputGuid;
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// traktor_mesh_BatchMeshEntityData_H
