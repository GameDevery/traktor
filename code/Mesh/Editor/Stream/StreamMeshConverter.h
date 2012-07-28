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
	virtual Ref< IMeshResource > createResource() const;

	virtual bool convert(
		const RefArray< model::Model >& models,
		const model::Model* occluderModel,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const std::vector< render::VertexElement >& vertexElements,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const;
};

	}
}

#endif	// traktor_mesh_StreamMeshConverter_H
