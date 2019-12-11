#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Render/VertexElement.h"

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

class VertexBuffer;
class IndexBuffer;

/*! Render mesh.
 * \ingroup Render
 */
class T_DLLCLASS Mesh : public Object
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		std::wstring name;
		Primitives primitives;
	};

	void setVertexElements(const AlignedVector< VertexElement >& vertexDeclaration);

	void setVertexBuffer(VertexBuffer* vertexBuffer);

	void setIndexBuffer(IndexBuffer* indexBuffer);

	void setParts(const AlignedVector< Part >& parts);

	void setBoundingBox(const Aabb3& boundingBox);

	const AlignedVector< VertexElement >& getVertexElements() const { return m_vertexElements; }

	VertexBuffer* getVertexBuffer() const { return m_vertexBuffer; }

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

	const AlignedVector< Part >& getParts() const { return m_parts; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	AlignedVector< VertexElement > m_vertexElements;
	Ref< VertexBuffer > m_vertexBuffer;
	Ref< IndexBuffer > m_indexBuffer;
	AlignedVector< Part > m_parts;
	Aabb3 m_boundingBox;
};

	}
}

