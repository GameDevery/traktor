/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SharedBufferHeapDx11_H
#define traktor_render_SharedBufferHeapDx11_H

#include <list>
#include <map>
#include "Core/Misc/ComRef.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! \brief Manages DX11 buffer memory.
 * \ingroup DX11
 *
 * This virtualizes the DX11 buffers from VertexBuffer instances; thus permit
 * larger chunks of memory being used and shared between multiple instances.
 *
 * This reduces the number of state changes required by the driver thus
 * improves performance.
 */
class SharedBufferHeapDx11 : public IBufferHeapDx11
{
	T_RTTI_CLASS;

public:
	SharedBufferHeapDx11(ContextDx11* context, const D3D11_BUFFER_DESC& dbd);

	virtual void destroy() override final;

	virtual bool alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk) override final;

	virtual void free(Chunk& chunk) override final;

private:
	struct FreeList
	{
		uint32_t offset;
		uint32_t size;

		bool operator < (const FreeList& rh) const;
	};

	struct Chain
	{
		ComRef< ID3D11Buffer > d3dBuffer;
		std::vector< FreeList > freeList;
	};

	Semaphore m_lock;
	ContextDx11* m_context;
	D3D11_BUFFER_DESC m_dbd;
	std::map< uint32_t, std::list< Chain > > m_chains;
};

	}
}

#endif	// traktor_render_SharedBufferHeapDx11_H
