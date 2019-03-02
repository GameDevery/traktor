#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/VertexElement.h"
#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/VertexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class IBufferHeapDx11;
class ContextDx11;

/*!
 * \ingroup DX11
 */
class VertexBufferStaticDx11 : public VertexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< VertexBufferStaticDx11 > create(
		ContextDx11* context,
		IBufferHeapDx11* bufferHeap,
		uint32_t bufferSize,
		const AlignedVector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferStaticDx11();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache) override final;

private:
	Ref< ContextDx11 > m_context;
	Ref< IBufferHeapDx11 > m_bufferHeap;
	IBufferHeapDx11::Chunk m_bufferChunk;
	AutoArrayPtr< uint8_t > m_data;

	VertexBufferStaticDx11(uint32_t bufferSize);
};

	}
}

