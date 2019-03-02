#pragma once

#include "Render/Mesh/MeshFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS SystemMeshFactory : public MeshFactory
{
	T_RTTI_CLASS;

public:
	virtual Ref< Mesh > createMesh(
		const AlignedVector< VertexElement >& vertexElements,
		uint32_t vertexBufferSize,
		IndexType indexType,
		uint32_t indexBufferSize
	) override final;
};

	}
}

