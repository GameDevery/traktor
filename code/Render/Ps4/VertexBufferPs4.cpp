#include "Render/Ps4/VertexBufferPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferPs4", VertexBufferPs4, VertexBuffer)

void VertexBufferPs4::prepare()
{
}

VertexBufferPs4::VertexBufferPs4(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}