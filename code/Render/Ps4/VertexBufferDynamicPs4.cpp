#include "Render/Ps4/VertexBufferDynamicPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicPs4", VertexBufferDynamicPs4, VertexBufferPs4)

Ref< VertexBufferDynamicPs4 > VertexBufferDynamicPs4::create(
	ContextPs4* context,
	uint32_t bufferSize,
	const AlignedVector< VertexElement >& vertexElements
)
{
	Ref< VertexBufferDynamicPs4 > vb = new VertexBufferDynamicPs4(bufferSize);
	vb->m_context = context;
	return vb;
}

VertexBufferDynamicPs4::~VertexBufferDynamicPs4()
{
	destroy();
}

void VertexBufferDynamicPs4::destroy()
{
}

void* VertexBufferDynamicPs4::lock()
{
	return 0;
}

void VertexBufferDynamicPs4::unlock()
{
}

void VertexBufferDynamicPs4::prepare()
{
	VertexBufferPs4::prepare();
}

VertexBufferDynamicPs4::VertexBufferDynamicPs4(uint32_t bufferSize)
:	VertexBufferPs4(bufferSize)
{
}

	}
}