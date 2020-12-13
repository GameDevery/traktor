#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/PipelineLayoutCache.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#if defined(_DEBUG)

const wchar_t* c_descriptorTypes[] =
{
	L"VK_DESCRIPTOR_TYPE_SAMPLER",
	L"VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
	L"VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE",
	L"VK_DESCRIPTOR_TYPE_STORAGE_IMAGE",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER",
	L"VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
	L"VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
	L"VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC",
	L"VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC",
	L"VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT"
};

std::wstring describe(const VkDescriptorSetLayoutCreateInfo& dlci)
{
	StringOutputStream ss;
	ss << L"bindingCount = " << dlci.bindingCount << Endl;
	for (uint32_t i = 0; i < dlci.bindingCount; ++i)
	{
		const auto& binding = dlci.pBindings[i];
		ss << L".pBindings[" << i << L"] = {" << Endl;
		ss << L"\t.binding = " << binding.binding << Endl;
		ss << L"\t.descriptorType = " << c_descriptorTypes[binding.descriptorType] << Endl;
		ss << L"}" << Endl;
	}
	return ss.str();
}

#endif

		}

PipelineLayoutCache::PipelineLayoutCache(VkDevice logicalDevice)
:	m_logicalDevice(logicalDevice)
{
}

PipelineLayoutCache::~PipelineLayoutCache()
{
	for (auto& it : m_entries)
	{
		vkDestroyDescriptorSetLayout(m_logicalDevice, it.second.descriptorSetLayout, 0);
		vkDestroyPipelineLayout(m_logicalDevice, it.second.pipelineLayout, 0);
	}
}

bool PipelineLayoutCache::get(uint32_t pipelineHash, const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
{
	auto it = m_entries.find(pipelineHash);
	if (it != m_entries.end())
	{
		outDescriptorSetLayout = it->second.descriptorSetLayout;
		outPipelineLayout = it->second.pipelineLayout;

#if defined(_DEBUG)
		std::wstring d = describe(dlci);
		if (d != it->second.debug)
		{
			log::error << L"Descriptor layout mismatch;" << Endl;
			log::error << L"cached:" << Endl;
			log::error << it->second.debug;
			log::error << L"requested:" << Endl;
			log::error << d;
			T_FATAL_ERROR;
		}
#endif
		return true;
	}
	else
	{
		if (vkCreateDescriptorSetLayout(m_logicalDevice, &dlci, nullptr, &outDescriptorSetLayout) != VK_SUCCESS)
			return false;

		VkPipelineLayoutCreateInfo lci = {};
		lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		lci.setLayoutCount = 1;
		lci.pSetLayouts = &outDescriptorSetLayout;
		lci.pushConstantRangeCount = 0;
		lci.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(m_logicalDevice, &lci, nullptr, &outPipelineLayout) != VK_SUCCESS)
			return false;

		auto& entry = m_entries[pipelineHash];
		entry.descriptorSetLayout = outDescriptorSetLayout;
		entry.pipelineLayout = outPipelineLayout;
#if defined(_DEBUG)
		entry.debug = describe(dlci);
#endif
		return true;
	}
}

	}
}
