#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class InternalVertexBuffer : public VertexBuffer
{
public:
	InternalVertexBuffer(int bufferSize)
	:	VertexBuffer(bufferSize)
	,	m_data(bufferSize)
	{
	}

	virtual void destroy() override final {}
	virtual void* lock() override final { return &m_data[0]; }
	virtual void unlock() override final {}

private:
	AlignedVector< uint8_t > m_data;
};

class InternalIndexBuffer : public IndexBuffer
{
public:
	InternalIndexBuffer(IndexType indexType, int bufferSize)
	:	IndexBuffer(indexType, bufferSize)
	,	m_data(bufferSize)
	{
	}

	virtual void destroy() override final {}
	virtual void* lock() override final { return &m_data[0]; }
	virtual void unlock() override final {}

private:
	AlignedVector< uint8_t > m_data;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SystemMeshFactory", SystemMeshFactory, MeshFactory)

Ref< Mesh > SystemMeshFactory::createMesh(
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize
) const
{
	Ref< VertexBuffer > vertexBuffer;
	Ref< IndexBuffer > indexBuffer;

	if (vertexBufferSize > 0)
		vertexBuffer = new InternalVertexBuffer(vertexBufferSize);

	if (indexBufferSize > 0)
		indexBuffer = new InternalIndexBuffer(indexType, indexBufferSize);

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexBuffer(indexBuffer);
	return mesh;
}

	}
}
