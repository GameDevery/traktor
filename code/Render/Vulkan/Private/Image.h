/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class CommandBuffer;
class Context;

/*!
 * \ingroup Vulkan
 */
class Image : public Object
{
	T_RTTI_CLASS;

public:
	Image() = delete;

	Image(const Image&) = delete;

	Image(Image&&) = default;

	explicit Image(Context* context);

	virtual ~Image();

	bool createSimple(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkFormat format,
		uint32_t usageBits
	);

	bool createCube(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkFormat format,
		uint32_t usageBits
	);

	bool createVolume(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t mipLevels,
		VkFormat format,
		uint32_t usageBits
	);

	bool createTarget(
		uint32_t width,
		uint32_t height,
		uint32_t multiSample,
		VkFormat format,
		VkImage swapChainImage
	);

	bool createDepthTarget(
		uint32_t width,
		uint32_t height,
		uint32_t multiSample,
		VkFormat format,
		bool usedAsTexture
	);

	void destroy();

	void* lock();

	void unlock();

	bool changeLayout(
		CommandBuffer* commandBuffer,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectMask,
		uint32_t mipLevel,
		uint32_t mipCount,
		uint32_t layerLevel,
		uint32_t layerCount
	);

	VkImage getVkImage() const { return m_image; }

	VkImageLayout getVkImageLayout(uint32_t mipLevel, uint32_t layerLevel) const { return m_imageLayouts[layerLevel * m_mipCount + mipLevel]; }

	VkImageView getVkImageView() const { return m_imageView; }

	uint32_t getResourceIndex() const { return m_resourceIndex; }

private:
	Context* m_context = nullptr;
	VmaAllocation m_allocation = 0;
	VkImage m_image = 0;
	VkImageView m_imageView = 0;
	AlignedVector< VkImageLayout > m_imageLayouts;
	uint32_t m_mipCount = 0;
	uint32_t m_layerCount = 0;
	uint32_t m_resourceIndex = ~0U;
	void* m_locked = nullptr;

	bool updateBindlessResource(VkImageLayout imageLayout);
};
		
}
