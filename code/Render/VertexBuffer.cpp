#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBuffer", VertexBuffer, Object)

VertexBuffer::VertexBuffer(uint32_t bufferSize)
:	m_bufferSize(bufferSize)
,	m_contentValid(false)
{
}

uint32_t VertexBuffer::getBufferSize() const
{
	return m_bufferSize;
}

	}
}
