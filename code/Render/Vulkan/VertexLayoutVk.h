#pragma once

#include "Render/IVertexLayout.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class VertexLayoutVk : public IVertexLayout
{
	T_RTTI_CLASS;

public:
	explicit VertexLayoutVk(
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const { return m_vertexBindingDescription; }

	const AlignedVector< VkVertexInputAttributeDescription >& getVkVertexInputAttributeDescriptions() const { return m_vertexAttributeDescriptions; }

	uint32_t getHash() const { return m_hash; }

	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
	uint32_t m_hash;
};

	}
}
