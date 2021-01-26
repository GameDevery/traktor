#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ISimpleTexture)

RenderTargetDepthVk::RenderTargetDepthVk(Context* context)
:	m_context(context)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::createPrimary(
	int32_t width,
	int32_t height,
	uint32_t multiSample,
	VkFormat format,
	const wchar_t* const tag
)
{
	m_image = new Image(m_context);
	m_image->createDepthTarget(width, height, multiSample, format, false);

	m_format = format;
	m_haveStencil = true;
	m_width = width;
	m_height = height;
	return true;
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag)
{
	VkFormat format;
	if (setDesc.ignoreStencil)
#if defined(__IOS__)
		format = VK_FORMAT_D16_UNORM;
#else
		format = VK_FORMAT_D32_SFLOAT;
#endif
	else
#if defined(__IOS__)
		format = VK_FORMAT_D16_UNORM_S8_UINT;
#else
		format = VK_FORMAT_D24_UNORM_S8_UINT;
#endif

	m_image = new Image(m_context);
	m_image->createDepthTarget(
		setDesc.width,
		setDesc.height,
		setDesc.multiSample,
		format,
		setDesc.usingDepthStencilAsTexture
	);

	m_format = format;
	m_haveStencil = !setDesc.ignoreStencil;
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetDepthVk::destroy()
{
	safeDestroy(m_image);
	m_context = nullptr;
}

ITexture* RenderTargetDepthVk::resolve()
{
	return this;
}

int32_t RenderTargetDepthVk::getMips() const
{
	return 1;
}

int32_t RenderTargetDepthVk::getWidth() const
{
	return m_width;
}

int32_t RenderTargetDepthVk::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetDepthVk::unlock(int32_t level)
{
}

void* RenderTargetDepthVk::getInternalHandle()
{
	return nullptr;
}

void RenderTargetDepthVk::prepareAsTarget(CommandBuffer* commandBuffer)
{
	m_image->changeLayout(
		commandBuffer,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		m_haveStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1
	);
}

void RenderTargetDepthVk::prepareAsTexture(CommandBuffer* commandBuffer)
{
	m_image->changeLayout(
		commandBuffer,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		m_haveStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1
	);
}

	}
}
