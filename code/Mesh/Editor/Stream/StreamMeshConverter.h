/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_StreamMeshConverter_H
#define traktor_mesh_StreamMeshConverter_H

#include "Mesh/Editor/IMeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class StreamMeshConverter : public IMeshConverter
{
public:
	virtual Ref< IMeshResource > createResource() const override final;

	virtual bool convert(
		const MeshAsset* meshAsset,
		const RefArray< model::Model >& models,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const AlignedVector< render::VertexElement >& vertexElements,
		int32_t maxInstanceCount,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const override final;
};

	}
}

#endif	// traktor_mesh_StreamMeshConverter_H
