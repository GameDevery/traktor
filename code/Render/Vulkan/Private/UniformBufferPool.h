#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Private/UniformBufferChain.h"

namespace traktor
{
	namespace render
	{

class UniformBufferPool : public Object
{
public:
	void destroy();

	void recycle();

	void flush();

	bool allocate(uint32_t size, UniformBufferRange& outRange);

	void free(const UniformBufferRange& range);

private:
	friend class Context;

	const wchar_t* const m_name;
	Context* m_context = nullptr;
	SmallMap< uint32_t, RefArray< UniformBufferChain > > m_chains;
	AlignedVector< UniformBufferRange > m_frees[4];
	uint32_t m_blockCount = 0;
	uint32_t m_count = 0;

	explicit UniformBufferPool(Context* context, uint32_t blockCount, const wchar_t* const name);
};

	}
}
