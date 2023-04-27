/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Thread/Atomic.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/TextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/PipelineLayoutCache.h"
#include "Render/Vulkan/Private/ShaderModuleCache.h"
#include "Render/Vulkan/Private/Utilities.h"

#undef max

namespace traktor::render
{
	namespace
	{

render::Handle s_handleTargetSize(L"_vk_targetSize");

VkShaderStageFlags getShaderStageFlags(uint8_t resourceStages)
{
	VkShaderStageFlags flags = 0;
	if (resourceStages & ProgramResourceVk::BsVertex)
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (resourceStages & ProgramResourceVk::BsFragment)
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (resourceStages & ProgramResourceVk::BsCompute)
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	T_ASSERT(flags != 0);
	return flags;
}

#if defined(_DEBUG)
std::wstring getShaderStageNames(uint8_t resourceStages)
{
	std::wstring names = L"";
	if (resourceStages & ProgramResourceVk::BsVertex)
		names += L"+vertex";
	if (resourceStages & ProgramResourceVk::BsFragment)
		names += L"+fragment";
	if (resourceStages & ProgramResourceVk::BsCompute)
		names += L"+compute";
	return names;
}
#endif

bool storeIfNotEqual(const float* source, int32_t length, float* dest)
{
	for (int32_t i = 0; i < length; ++i)
	{
		if (dest[i] != source[i])
		{
			for (; i < length; ++i)
				dest[i] = source[i];
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Vector4* source, int32_t length, float* dest)
{
	for (int32_t i = 0; i < length; ++i)
	{
		if (Vector4::loadAligned(&dest[i * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4]);
			return true;
		}
	}
	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramVk", ProgramVk, IProgram)

ProgramVk::ProgramVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

ProgramVk::~ProgramVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool ProgramVk::create(
	ShaderModuleCache* shaderModuleCache,
	PipelineLayoutCache* pipelineLayoutCache,
	const ProgramResourceVk* resource,
	int32_t maxAnistropy,
	float mipBias,
	const wchar_t* const tag
)
{
	VkShaderStageFlags stageFlags;

	m_tag = tag;
	m_renderState = resource->m_renderState;
	m_shaderHash = resource->m_shaderHash;

	// Get shader modules.
	if (!resource->m_vertexShader.empty() && !resource->m_fragmentShader.empty())
	{
		T_FATAL_ASSERT(resource->m_computeShader.empty());

		if ((m_vertexShaderModule = shaderModuleCache->get(resource->m_vertexShader, resource->m_vertexShaderHash)) == 0)
			return false;
		if ((m_fragmentShaderModule = shaderModuleCache->get(resource->m_fragmentShader, resource->m_fragmentShaderHash)) == 0)
			return false;

		stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (!resource->m_computeShader.empty())
	{
		if ((m_computeShaderModule = shaderModuleCache->get(resource->m_computeShader, resource->m_computeShaderHash)) == 0)
			return false;

		stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	}
    else
        return false;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_context->getPhysicalDevice(), &deviceProperties);
	uint32_t uniformBufferOffsetAlignment = std::max< uint32_t >(
		(uint32_t)deviceProperties.limits.minUniformBufferOffsetAlignment,
		sizeof(intptr_t)
	);

	// Create descriptor set layouts for shader uniforms.
	AlignedVector< VkDescriptorSetLayoutBinding  > dslb;

	// Each program has 3 uniform buffer bindings (Once, Frame and Draw cbuffers).
	for (int32_t i = 0; i < 3; ++i)
	{
		if (resource->m_uniformBufferSizes[i] == 0)
			continue;

		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = i;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		lb.descriptorCount = 1;
		lb.stageFlags = stageFlags;
	}

	// Append sampler bindings.
	for (const auto& sampler : resource->m_samplers)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = sampler.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		lb.descriptorCount = 1;
		lb.stageFlags = getShaderStageFlags(sampler.stages);
	}

	// Append texture bindings.
	for (const auto& texture : resource->m_textures)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = texture.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		lb.descriptorCount = 1;
		lb.stageFlags = getShaderStageFlags(texture.stages);
	}

	// Append image bindings.
	for (const auto& image : resource->m_images)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = image.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		lb.descriptorCount = 1;
		lb.stageFlags = getShaderStageFlags(image.stages);		
	}

	// Append sbuffer bindings.
	for (const auto& sbuffer : resource->m_sbuffers)
	{
		auto& lb = dslb.push_back();
		lb = {};
		lb.binding = sbuffer.binding;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		lb.descriptorCount = 1;
		lb.stageFlags = getShaderStageFlags(sbuffer.stages);
	}

	VkDescriptorSetLayoutCreateInfo dlci = {};
	dlci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dlci.pNext = nullptr;
	dlci.bindingCount = (uint32_t)dslb.size();
	dlci.pBindings = dslb.c_ptr();

	if (!pipelineLayoutCache->get(resource->m_layoutHash, dlci, m_descriptorSetLayout, m_pipelineLayout))
		return false;

	// Create uniform buffer CPU side shadow data.
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_uniformBuffers[i].size = resource->m_uniformBufferSizes[i] * sizeof(float);
		if (m_uniformBuffers[i].size == 0)
			continue;

		m_uniformBuffers[i].alignedSize = alignUp(m_uniformBuffers[i].size, uniformBufferOffsetAlignment);
		m_uniformBuffers[i].data.resize(resource->m_uniformBufferSizes[i], 0.0f);
	}

	// Create samplers.
	for (const auto& resourceSampler : resource->m_samplers)
	{
		VkSamplerCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sci.magFilter = c_filters[resourceSampler.state.magFilter];
		sci.minFilter = c_filters[resourceSampler.state.minFilter];
		sci.mipmapMode = c_mipMapModes[resourceSampler.state.mipFilter];
		sci.addressModeU = c_addressModes[resourceSampler.state.addressU];
		sci.addressModeV = c_addressModes[resourceSampler.state.addressV];
		sci.addressModeW = c_addressModes[resourceSampler.state.addressW];
		sci.mipLodBias = resourceSampler.state.mipBias + mipBias;
		
		if (maxAnistropy > 0)
			sci.anisotropyEnable = resourceSampler.state.useAnisotropic ? VK_TRUE : VK_FALSE;
		else
			sci.anisotropyEnable = VK_FALSE;

		sci.maxAnisotropy = (float)maxAnistropy;
		sci.compareEnable = (resourceSampler.state.compare != CfNone) ? VK_TRUE : VK_FALSE;
		sci.compareOp = c_compareOperations[resourceSampler.state.compare];
		sci.minLod = 0.0f;
		sci.maxLod = resourceSampler.state.ignoreMips ? 0.0f : std::numeric_limits< float >::max();
		sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		sci.unnormalizedCoordinates = VK_FALSE;

		VkSampler sampler = pipelineLayoutCache->getSampler(sci);
		if (!sampler)
			return false;

		m_samplers.push_back({ resourceSampler.binding, sampler });
	}

	// Create textures.
	for (const auto& resourceTexture : resource->m_textures)
	{
#if !defined(_DEBUG)
		m_textures.push_back({ resourceTexture.binding });
#else
		m_textures.push_back({ resourceTexture.name + L" (" + getShaderStageNames(resourceTexture.stages) + L")", resourceTexture.binding });
#endif
	}

	// Create images.
	for (const auto& resourceImage : resource->m_images)
	{
#if !defined(_DEBUG)
		m_images.push_back({ resourceImage.binding });
#else
		m_images.push_back({ resourceImage.name + L" (" + getShaderStageNames(resourceImage.stages) + L")", resourceImage.binding });
#endif
	}

	// Create sbuffers.
	for (const auto& resourceSBuffer : resource->m_sbuffers)
	{
#if !defined(_DEBUG)
		m_sbuffers.push_back({ resourceSBuffer.binding });
#else
		m_sbuffers.push_back({ resourceSBuffer.name + L" (" + getShaderStageNames(resourceSBuffer.stages) + L")", resourceSBuffer.binding });
#endif
	}

	// Setup parameter mapping.
	for (auto p : resource->m_parameters)
	{
		auto& pm = m_parameterMap[getParameterHandle(p.name)];
#if defined(_DEBUG)
		pm.name = p.name;
#endif
		pm.buffer = p.buffer;
		pm.offset = p.offset;
		pm.size = p.size;
	}

	return true;
}

bool ProgramVk::validateGraphics(
	CommandBuffer* commandBuffer,
	const float targetSize[2]
)
{
	// Set implicit parameters.
	setVectorParameter(
		s_handleTargetSize,
		Vector4(targetSize[0], targetSize[1], 0.0f, 0.0f)
	);

	// Update content of uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size || !m_uniformBuffers[i].dirty)
			continue;

		auto pool = m_context->getUniformBufferPool(i);

		if (m_uniformBuffers[i].range.ptr)
			pool->free(m_uniformBuffers[i].range);

		if (!pool->allocate(m_uniformBuffers[i].alignedSize, m_uniformBuffers[i].range))
		{
			log::error << L"Out of uniform buffer pool memory (" << m_tag << L")!" << Endl;
			return false;
		}

		std::memcpy(
			m_uniformBuffers[i].range.ptr,
			m_uniformBuffers[i].data.c_ptr(),
			m_uniformBuffers[i].size
		);

		m_uniformBuffers[i].dirty = false;
	}

	if (!validateDescriptorSet())
		return false;

	// Get offsets into buffers.
	StaticVector< uint32_t, 3+8 > bufferOffsets;
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;
		bufferOffsets.push_back(m_uniformBuffers[i].range.offset);
	}
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.bufferView)
			continue;
		auto bvvk = sbuffer.bufferView;
		bufferOffsets.push_back(bvvk->getVkBufferOffset());
	}

	vkCmdBindDescriptorSets(
		*commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1, &m_descriptorSet,
		(uint32_t)bufferOffsets.size(), bufferOffsets.c_ptr()
	);


	const VkDescriptorSet bindlessDescriptorSet = m_context->getBindlessDescriptorSet();
	vkCmdBindDescriptorSets(
		*commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		1,
		1, &bindlessDescriptorSet,
		0, nullptr
	);


	if (m_renderState.stencilEnable)
		vkCmdSetStencilReference(
			*commandBuffer,
			VK_STENCIL_FRONT_AND_BACK,
			m_stencilReference
		);

	return true;
}

bool ProgramVk::validateCompute(CommandBuffer* commandBuffer)
{
	// Update content of uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size || !m_uniformBuffers[i].dirty)
			continue;

		auto pool = m_context->getUniformBufferPool(i);

		if (m_uniformBuffers[i].range.ptr)
			pool->free(m_uniformBuffers[i].range);

		if (!pool->allocate(m_uniformBuffers[i].alignedSize, m_uniformBuffers[i].range))
		{
			log::error << L"Out of uniform buffer pool memory (" << m_tag << L")!" << Endl;
			return false;
		}

		std::memcpy(
			m_uniformBuffers[i].range.ptr,
			m_uniformBuffers[i].data.c_ptr(),
			m_uniformBuffers[i].size
		);

		m_uniformBuffers[i].dirty = false;
	}

	if (!validateDescriptorSet())
		return false;

	// Get offsets into buffers.
	StaticVector< uint32_t, 3+8 > bufferOffsets;
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;
		bufferOffsets.push_back(m_uniformBuffers[i].range.offset);
	}
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.bufferView)
			continue;
		auto bvvk = sbuffer.bufferView;
		bufferOffsets.push_back(bvvk->getVkBufferOffset());
	}

	vkCmdBindDescriptorSets(
		*commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		m_pipelineLayout,
		0,
		1, &m_descriptorSet,
		(uint32_t)bufferOffsets.size(), bufferOffsets.c_ptr()
	);


	const VkDescriptorSet bindlessDescriptorSet = m_context->getBindlessDescriptorSet();
	vkCmdBindDescriptorSets(
		*commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		m_pipelineLayout,
		1,
		1, &bindlessDescriptorSet,
		0, nullptr
	);


	return true;
}

void ProgramVk::destroy()
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (m_uniformBuffers[i].range.ptr)
			m_context->getUniformBufferPool(i)->free(m_uniformBuffers[i].range);
		m_uniformBuffers[i].range.ptr = nullptr;
	}

	for (auto it : m_descriptorSets)
	{
		m_context->addDeferredCleanup([
			descriptorPoolRevision = m_descriptorPoolRevision,
			descriptorSet = it.second
		](Context* cx) {
			if (cx->getDescriptorPoolRevision() == descriptorPoolRevision)
				vkFreeDescriptorSets(cx->getLogicalDevice(), cx->getDescriptorPool(), 1, &descriptorSet);
		});
	}
	m_descriptorSets.clear();

	m_vertexShaderModule = 0;
	m_fragmentShaderModule = 0;
	m_computeShaderModule = 0;

	m_descriptorSetLayout = 0;
	m_pipelineLayout = 0;

	m_samplers.clear();
	m_textures.clear();
	m_images.clear();
	m_sbuffers.clear();
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(&param, 1, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramVk::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramVk::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 4 <= (int)i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		param.storeAligned(&ub.data[i->second.offset]);
		ub.dirty = true;
	}
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 16 <= (int)i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&ub.data[i->second.offset + j * 16]);
		ub.dirty = true;
	}
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_textures[i->second.offset].texture = texture;
}

void ProgramVk::setImageViewParameter(handle_t handle, ITexture* imageView)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_images[i->second.offset].texture = imageView;
}

void ProgramVk::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_sbuffers[i->second.offset].bufferView = (const BufferViewVk*)bufferView;
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

bool ProgramVk::validateDescriptorSet()
{
	// Ensure we're still using same descriptor pool revision; if not then we need to flush our cached descriptor sets.
	if (m_context->getDescriptorPoolRevision() != m_descriptorPoolRevision)
	{
		m_descriptorSets.reset();
		m_descriptorPoolRevision = m_context->getDescriptorPoolRevision();
	}

	// Create key from current bound resources.
	DescriptorSetKey key;
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;
		key.push_back((intptr_t)m_uniformBuffers[i].range.chain);
	}	
	//for (const auto& texture : m_textures)
	//{
	//	if (!texture.texture)
	//		return false;
	//	auto resolved = texture.texture->resolve();
	//	if (!resolved)
	//		return false;
	//	key.push_back((intptr_t)resolved);
	//}
	for (const auto& image : m_images)
	{
		if (!image.texture)
			return false;
		auto resolved = image.texture->resolve();
		if (!resolved)
			return false;
		key.push_back((intptr_t)resolved);
	}
	for (const auto& sbuffer : m_sbuffers)
	{
		if (!sbuffer.bufferView)
			return false;
		key.push_back((intptr_t)sbuffer.bufferView->getVkBuffer());
	}

	// Get already created descriptor set for bound resources.
	//auto it = m_descriptorSets.find(key);
	//if (it != m_descriptorSets.end())
	//{
	//	m_descriptorSet = it->second;
	//	return true;
	//}
	for (const auto& k : m_descriptorSets)
	{
		if (k.first.size() == key.size())
		{
			if (memcmp(k.first.c_ptr(), key.c_ptr(), sizeof(intptr_t) * key.size()) == 0)
			{
				m_descriptorSet = k.second;
				return true;
			}
		}
	}

	// No such descriptor set found, need to create another set.
	VkDescriptorSetAllocateInfo dsai;
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.pNext = nullptr;
	dsai.descriptorPool = m_context->getDescriptorPool();
	dsai.descriptorSetCount = 1;
	dsai.pSetLayouts = &m_descriptorSetLayout;
	if (vkAllocateDescriptorSets(m_context->getLogicalDevice(), &dsai, &m_descriptorSet) != VK_SUCCESS)
	{
		log::error << L"Unable to allocate Vulkan descriptor set (" << m_tag << L")!" << Endl;
		m_descriptorSets.insert(key, 0);
		return false;
	}

	StaticVector< VkDescriptorBufferInfo, 16 > bufferInfos;
	StaticVector< VkDescriptorImageInfo, 32 + 32 > imageInfos;
	StaticVector< VkWriteDescriptorSet, 16 + 32 + 32 > writes;

	// Add uniform buffer bindings.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = *m_uniformBuffers[i].range.chain->getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_uniformBuffers[i].size;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = i;
	}

	// Add sampler bindings.
	for (const auto& sampler : m_samplers)
	{
		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = sampler.sampler;
		imageInfo.imageView = 0;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sampler.binding;
	}

	// Add texture bindings.
	//for (const auto& texture : m_textures)
	//{
	//	T_ASSERT(texture.texture);
	//	auto resolved = texture.texture->resolve();
	//	T_ASSERT(resolved);

	//	auto& imageInfo = imageInfos.push_back();
	//	imageInfo.sampler = 0;

	//	if (is_a< TextureVk >(resolved))
	//	{
	//		imageInfo.imageView = static_cast< TextureVk* >(resolved)->getImage().getVkImageView();
	//		imageInfo.imageLayout = static_cast< TextureVk* >(resolved)->getImage().getVkImageLayout(0, 0); // \fixme Assuming all mips have same layout at this point.
	//	}
	//	else if (is_a< RenderTargetVk >(resolved))
	//	{
	//		imageInfo.imageView = static_cast< RenderTargetVk* >(resolved)->getImageResolved()->getVkImageView();
	//		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//	}
	//	else if (is_a< RenderTargetDepthVk >(resolved))
	//	{
	//		imageInfo.imageView = static_cast< RenderTargetDepthVk* >(resolved)->getImage()->getVkImageView();
	//		imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	//	}

	//	T_ASSERT(imageInfo.imageView != 0);

	//	auto& write = writes.push_back();
	//	write = {};
	//	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	write.pNext = nullptr;
	//	write.dstSet = m_descriptorSet;
	//	write.descriptorCount = 1;
	//	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	//	write.pImageInfo = &imageInfo;
	//	write.dstArrayElement = 0;
	//	write.dstBinding = texture.binding;
	//}

	// Add image bindings.
	for (const auto& image : m_images)
	{
		T_ASSERT(image.texture);
		auto resolved = image.texture->resolve();
		T_ASSERT(resolved);

		auto& imageInfo = imageInfos.push_back();
		imageInfo.sampler = 0;

		if (is_a< TextureVk >(resolved))
		{
			imageInfo.imageView = static_cast< TextureVk* >(resolved)->getImage().getVkImageView();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		T_ASSERT(imageInfo.imageView != 0);

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = 0;
		write.dstBinding = image.binding;
	}

	// Add sbuffer bindings.
	for (const auto& sbuffer : m_sbuffers)
	{
		T_ASSERT(sbuffer.bufferView);
		auto bvvk = sbuffer.bufferView;

		auto& bufferInfo = bufferInfos.push_back();
		bufferInfo.buffer = bvvk->getVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = bvvk->getVkBufferRange();

		auto& write = writes.push_back();
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		write.pBufferInfo = &bufferInfo;
		write.dstArrayElement = 0;
		write.dstBinding = sbuffer.binding;
	}

	vkUpdateDescriptorSets(
		m_context->getLogicalDevice(),
		(uint32_t)writes.size(),
		writes.c_ptr(),
		0,
		nullptr
	);

	m_descriptorSets.insert(key, m_descriptorSet);
	return true;
}

bool ProgramVk::DescriptorSetKey::operator < (const DescriptorSetKey& rh) const
{
	if (size() < rh.size())
		return true;
	if (size() > rh.size())
		return false;

	for (uint32_t i = 0; i < size(); ++i)
	{
		if ((*this)[i] < rh[i])
			return true;
	}

	return false;
}

bool ProgramVk::DescriptorSetKey::operator > (const DescriptorSetKey& rh) const
{
	if (size() < rh.size())
		return false;
	if (size() > rh.size())
		return true;

	for (uint32_t i = 0; i < size(); ++i)
	{
		if ((*this)[i] > rh[i])
			return true;
	}

	return false;
}

}
