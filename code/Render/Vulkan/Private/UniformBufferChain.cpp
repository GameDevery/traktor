#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/Private/Buffer.h"
#include "Render/Vulkan/Private/UniformBufferChain.h"

namespace traktor
{
	namespace render
	{

Ref< UniformBufferChain > UniformBufferChain::create(Context* context, uint32_t blockCount, uint32_t blockSize)
{
	Ref< Buffer > buffer = new Buffer(context);
	if (!buffer->create(
		blockCount * blockSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		true,
		true
	))
	{
		buffer->destroy();
		return nullptr;
	}

	void* top = buffer->lock();
	if (!top)
	{
		buffer->destroy();
		return nullptr;
	}

	return new UniformBufferChain(buffer, top, blockCount, blockSize);
}

void UniformBufferChain::destroy()
{
	safeDestroy(m_buffer);
}

bool UniformBufferChain::allocate(UniformBufferRange& outRange)
{
	uint8_t* ptr = (uint8_t*)m_allocator.alloc();
	if (!ptr)
		return false;

	outRange.chain = this;
	outRange.offset = (uint32_t)(ptr - (uint8_t*)m_allocator.top());
	outRange.ptr = ptr;
	return true;
}

void UniformBufferChain::free(const UniformBufferRange& range)
{
	T_ASSERT(range.chain == this);
	m_allocator.free(range.ptr);
}

UniformBufferChain::UniformBufferChain(Buffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize)
:	m_buffer(buffer)
,	m_allocator(top, blockCount, blockSize)
{
}

	}
}