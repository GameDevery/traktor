#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetVk", RenderTargetVk, ISimpleTexture)

RenderTargetVk::RenderTargetVk(Context* context)
:	m_context(context)
{
}

RenderTargetVk::~RenderTargetVk()
{
	destroy();
}

bool RenderTargetVk::createPrimary(
	int32_t width,
	int32_t height,
	uint32_t multiSample,
	VkFormat format,
	VkImage swapChainImage,
	const wchar_t* const tag
)
{
	if (multiSample > 1)
	{
		m_imageTarget = new Image(m_context);
		if (!m_imageTarget->createTarget(width, height, multiSample, format, 0))
			return false;

		m_imageResolved = new Image(m_context);
		if (!m_imageResolved->createTarget(width, height, 1, format, swapChainImage))
			return false;
	}
	else
	{
		m_imageTarget = new Image(m_context);
		if (!m_imageTarget->createTarget(width, height, 0, format, swapChainImage))
			return false;

		m_imageResolved = m_imageTarget;
	}

	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_imageTarget->getVkImage(), VK_OBJECT_TYPE_IMAGE);
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_imageResolved->getVkImage(), VK_OBJECT_TYPE_IMAGE);

	m_format = format;
	m_width = width;
	m_height = height;
	return true;
}

bool RenderTargetVk::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag)
{
	VkFormat format = determineSupportedTargetFormat(m_context->getPhysicalDevice(), desc.format);
	if (format == VK_FORMAT_UNDEFINED)
		return false;

	m_imageTarget = new Image(m_context);
	if (!m_imageTarget->createTarget(setDesc.width, setDesc.height, setDesc.multiSample, format, 0))
		return false;

	if (setDesc.multiSample > 1)
	{
		m_imageResolved = new Image(m_context);
		if (!m_imageResolved->createTarget(setDesc.width, setDesc.height, 1, format, 0))
			return false;
	}
	else
		m_imageResolved = m_imageTarget;

	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_imageTarget->getVkImage(), VK_OBJECT_TYPE_IMAGE);
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_imageResolved->getVkImage(), VK_OBJECT_TYPE_IMAGE);

	m_format = format;
	m_width = setDesc.width;
	m_height = setDesc.height;

	// Set layout to be read by shader initially since we cannot ensure
	// target is being used as target directly.
	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
	m_imageResolved->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
	commandBuffer->submitAndWait();

	return true;
}

void RenderTargetVk::destroy()
{
	safeDestroy(m_imageTarget);
	safeDestroy(m_imageResolved);
	m_context = nullptr;
}

ITexture* RenderTargetVk::resolve()
{
	return this;
}

int32_t RenderTargetVk::getMips() const
{
	return 1;
}

int32_t RenderTargetVk::getWidth() const
{
	return m_width;
}

int32_t RenderTargetVk::getHeight() const
{
	return m_height;
}

bool RenderTargetVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetVk::unlock(int32_t level)
{
}

void* RenderTargetVk::getInternalHandle()
{
	return nullptr;
}

void RenderTargetVk::prepareForPresentation(CommandBuffer* commandBuffer)
{
	m_imageResolved->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
}

void RenderTargetVk::prepareAsTarget(CommandBuffer* commandBuffer)
{
	m_imageTarget->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
	if (m_imageResolved != m_imageTarget)
		m_imageResolved->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
}

void RenderTargetVk::prepareAsTexture(CommandBuffer* commandBuffer)
{
	m_imageResolved->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
}

void RenderTargetVk::prepareForReadBack(CommandBuffer* commandBuffer)
{
	m_imageResolved->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
}

	}
}
