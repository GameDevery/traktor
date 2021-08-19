#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Memory/BlockAllocator.h"

namespace traktor
{
	namespace render
	{

class ApiBuffer;
class Context;
class UniformBufferChain;

struct UniformBufferRange
{
	UniformBufferChain* chain = nullptr;
	uint32_t offset = 0;
	void* ptr = nullptr;
};

class UniformBufferChain : public Object
{
public:
	static Ref< UniformBufferChain > create(Context* context, uint32_t blockCount, uint32_t blockSize);

	void destroy();

	bool allocate(UniformBufferRange& outRange);

	void free(const UniformBufferRange& range);

	ApiBuffer* getBuffer() const { return m_buffer; }

private:
	Ref< ApiBuffer > m_buffer;
	BlockAllocator m_allocator;

	explicit UniformBufferChain(ApiBuffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize);
};

	}
}
