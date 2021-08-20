#pragma once

#include "Render/IBufferView.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class BufferViewVk : public IBufferView
{
	T_RTTI_CLASS;

public:
	BufferViewVk() = default;

	explicit BufferViewVk(VkBuffer buffer, uint32_t offset, uint32_t range, uint32_t size);

	VkBuffer getVkBuffer() const { return m_buffer; }

	uint32_t getVkBufferOffset() const { return m_offset; }

	uint32_t getVkBufferRange() const { return m_range; }

	uint32_t getVkBufferSize() const { return m_size; }

	bool operator == (const BufferViewVk& rh) const {
		return m_buffer == rh.m_buffer && m_offset == rh.m_offset;
	}

	bool operator != (const BufferViewVk& rh) const {
		return m_buffer != rh.m_buffer || m_offset != rh.m_offset;
	}

private:
	VkBuffer m_buffer = 0;
	uint32_t m_offset = 0;
	uint32_t m_range = 0;
	uint32_t m_size = 0;
};

	}
}
