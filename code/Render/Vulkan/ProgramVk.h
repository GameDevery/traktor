/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramVk_H
#define traktor_render_ProgramVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ProgramResourceVk;

/*!
 * \ingroup Vulkan
 */
class ProgramVk : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramVk();

	virtual ~ProgramVk();

	bool create(VkPhysicalDevice physicalDevice, VkDevice device, const ProgramResourceVk* resource);

	bool validate(VkDevice device, VkDescriptorSet descriptorSet);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	const RenderState& getRenderState() const { return m_renderState; }

	VkShaderModule getVertexVkShaderModule() const { return m_vertexShaderModule; }

	VkShaderModule getFragmentVkShaderModule() const { return m_fragmentShaderModule; }

private:
	struct ParameterMap
	{
		uint32_t offset;	//!< Offset into m_parameterScalarData
		uint32_t size;		//!< Number of floats.
		uint32_t uniformBufferOffset;

		ParameterMap()
		:	offset(0)
		,	size(0)
		,	uniformBufferOffset(0)
		{
		}
	};

	struct DeviceBuffer
	{
		VkBuffer buffer;
		VkDeviceMemory memory;

		DeviceBuffer()
		:	buffer(0)
		,	memory(0)
		{
		}
	};

	struct UniformBuffer
	{
		AlignedVector< DeviceBuffer > deviceBuffers;
		uint32_t size;
		uint32_t updateCount;
		AlignedVector< ParameterMap > parameters;

		UniformBuffer()
		:	size(0)
		,	updateCount(0)
		{
		}
	};

	RenderState m_renderState;

	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
	
	UniformBuffer m_vertexUniformBuffers[3];
	UniformBuffer m_fragmentUniformBuffers[3];

	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< float > m_parameterScalarData;
};

	}
}

#endif	// traktor_render_ProgramVk_H
