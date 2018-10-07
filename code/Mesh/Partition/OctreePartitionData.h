/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_OctreePartitionData_H
#define traktor_mesh_OctreePartitionData_H

#include "Core/Containers/AlignedVector.h"
#include "Mesh/Partition/IPartitionData.h"

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

class OctreeNodeData;

class T_DLLCLASS OctreePartitionData : public IPartitionData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPartition > createPartition() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class PartitionMeshConverter;

	AlignedVector< std::wstring > m_worldTechniques;
	Ref< OctreeNodeData > m_nodeData;
};

	}
}

#endif	// traktor_mesh_OctreePartitionData_H
