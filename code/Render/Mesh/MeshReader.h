#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace render
	{

class MeshFactory;
class Mesh;

/*! \brief Render mesh reader.
 * \ingroup Render
 */
class T_DLLCLASS MeshReader : public Object
{
	T_RTTI_CLASS;

public:
	MeshReader(MeshFactory* meshFactory);

	Ref< Mesh > read(IStream* stream) const;

private:
	Ref< MeshFactory > m_meshFactory;
};

	}
}

