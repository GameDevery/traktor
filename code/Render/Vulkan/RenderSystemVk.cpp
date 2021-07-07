#if defined(__ANDROID__)
#	include <android_native_app_glue.h>
#endif
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/VertexElement.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderSystemVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/StructBufferDynamicVk.h"
#include "Render/Vulkan/StructBufferStaticVk.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/PipelineLayoutCache.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/ShaderModuleCache.h"
#include "Render/Vulkan/Private/Utilities.h"
#include "Render/Vulkan/Private/VertexAttributes.h"

#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/Vulkan/Linux/Window.h"
#elif defined(__ANDROID__)
#	include "Core/System/Android/DelegateInstance.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/Utilities.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

const char* c_validationLayerNames[] = { "VK_LAYER_KHRONOS_validation", nullptr };
#if defined(_WIN32)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_utils", "VK_KHR_get_physical_device_properties2" };
#elif defined(__LINUX__) || defined(__RPI__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_xlib_surface", "VK_EXT_debug_utils", "VK_KHR_get_physical_device_properties2" };
#elif defined(__ANDROID__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_android_surface" };
#elif defined(__MAC__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_EXT_metal_surface", "VK_EXT_debug_utils", "VK_KHR_get_physical_device_properties2" };
#else
const char* c_extensions[] = { "VK_KHR_surface", "VK_EXT_debug_utils", "VK_KHR_get_physical_device_properties2" };
#endif

#if defined(__ANDROID__) || defined(__RPI__)
const char* c_deviceExtensions[] = { "VK_KHR_swapchain" };
#else
const char* c_deviceExtensions[] =
{
	"VK_KHR_swapchain",
	"VK_KHR_storage_buffer_storage_class",	// Required by VK_KHR_16bit_storage, VK_KHR_8bit_storage and VK_KHR_shader_float16_int8.
	"VK_KHR_16bit_storage",
	"VK_KHR_8bit_storage",
	"VK_KHR_shader_float16_int8"
};
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	if (pCallbackData && pCallbackData->pMessage)
	{
		log::info << mbstows(pCallbackData->pMessage) << Endl;
	}
	return VK_FALSE;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties dp;
    // VkPhysicalDeviceFeatures df;
    
	vkGetPhysicalDeviceProperties(device, &dp);
    // vkGetPhysicalDeviceFeatures(device, &df);

	if (dp.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		return false;

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
#if defined(__LINUX__) || defined(__RPI__)
:	m_display(nullptr)
,	m_instance(0)
#else
:	m_instance(0)
#endif
,	m_physicalDevice(0)
,	m_logicalDevice(0)
,	m_debugMessenger(0)
,	m_allocator(0)
,	m_maxAnisotropy(0)
,	m_mipBias(0.0f)
{
#if defined(__ANDROID__)
	m_screenWidth = 0;
	m_screenHeight = 0;
#endif
}

bool RenderSystemVk::create(const RenderSystemDesc& desc)
{
	VkResult result;

#if defined(__LINUX__) || defined(__RPI__)
	if ((m_display = XOpenDisplay(0)) == nullptr)
	{
		log::error << L"Unable to create Vulkan renderer; Failed to open X display" << Endl;
		return false;
	}
#elif defined(__ANDROID__)
	auto app = desc.sysapp.instance->getApplication();
	m_screenWidth = ANativeWindow_getWidth(app->window);
	m_screenHeight = ANativeWindow_getHeight(app->window);
#endif

	if (!initializeVulkanApi())
	{
		log::error << L"Unable to create Vulkan renderer; Failed to initialize Vulkan API." << Endl;
		return false;
	}

	// Check for available layers.
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);

	AutoArrayPtr< VkLayerProperties > layersAvailable(new VkLayerProperties[layerCount]);
	vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable.ptr());

	AlignedVector< const char* > validationLayers;
	if (desc.validation)
	{
		for (uint32_t i = 0; i < layerCount; ++i)
		{
			bool found = false;
			for (uint32_t j = 0; c_validationLayerNames[j] != nullptr; ++j)
			{
				if (strcmp(layersAvailable[i].layerName, c_validationLayerNames[j]) == 0)
					found = true;
			}
			if (found)
				validationLayers.push_back(strdup(layersAvailable[i].layerName));
		}
	}

	// Create Vulkan instance.
	VkApplicationInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ai.pNext = nullptr;
	ai.pApplicationName = "Traktor";
	ai.pEngineName = "Traktor";
	ai.engineVersion = 1;
	ai.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ii.pApplicationInfo = &ai;
	ii.enabledLayerCount = (uint32_t)validationLayers.size();
	ii.ppEnabledLayerNames = validationLayers.c_ptr();
	ii.enabledExtensionCount = sizeof_array(c_extensions);
	ii.ppEnabledExtensionNames = c_extensions;

	if ((result = vkCreateInstance(&ii, 0, &m_instance)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan instance (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	// Load Vulkan extensions.
	if (!initializeVulkanExtensions(m_instance))
	{
		log::error << L"Failed to create Vulkan; failed to load extensions." << Endl;
		return false;
	}

#if !defined(__ANDROID__)
	if (desc.validation)
	{
		// Setup debug port callback.
		VkDebugUtilsMessengerCreateInfoEXT dumci = {};
		dumci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dumci.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		dumci.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT /*| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/;
		dumci.pfnUserCallback = debugCallback;

		if ((result = vkCreateDebugUtilsMessengerEXT(m_instance, &dumci, nullptr, &m_debugMessenger)) != VK_SUCCESS)
		{
			log::error << L"Failed to create Vulkan; failed to set debug report callback." << Endl;
			return false;
		}
	}
#endif

	// Select physical device.
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, 0);
	if (physicalDeviceCount == 0)
	{
		log::error << L"Failed to create Vulkan; no physical devices." << Endl;
		return false;
	}
	log::info << L"Found " << physicalDeviceCount << L" physical device(s)," << Endl;

	AutoArrayPtr< VkPhysicalDevice > physicalDevices(new VkPhysicalDevice[physicalDeviceCount]);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.ptr());
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties pdp = {};
		vkGetPhysicalDeviceProperties(physicalDevices[i], &pdp);

		if (isDeviceSuitable(physicalDevices[i]))
		{
			if (!m_physicalDevice)
				m_physicalDevice = physicalDevices[i];
		}

		log::info << (i + 1) << L". \"" << mbstows(pdp.deviceName) << L"\"";
		
		if (m_physicalDevice == physicalDevices[i])
			log::info << L" (selected)";
		
		log::info << Endl;
	}

	if (!m_physicalDevice)
	{
		log::warning << L"Unable to find a suitable device; attempting to use first reported." << Endl;
		m_physicalDevice = physicalDevices[0];
	}

	// Get physical device graphics queue.
	uint32_t graphicsQueueIndex = ~0;
	uint32_t computeQueueIndex = ~0;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, 0);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.ptr());

	// Select graphics queue.
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			graphicsQueueIndex = i;
			break;
		}
	}

	// Select compute queue.
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
		{
#if !defined(__RPI__)
			// Ensure family can support two queues.
			if (i == graphicsQueueIndex)
			{
				if (queueFamilyProperties[i].queueCount <= 1)
					continue;
			}
#endif
			computeQueueIndex = i;
			break;
		}
	}
	if (graphicsQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable graphics queue found." << Endl;
		return false;
	}
	if (computeQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable compute queue found." << Endl;
		return false;
	}

	// Create logical device.
	const float queuePriorities[] = { 1.0f, 1.0f };

    VkDeviceQueueCreateInfo dqci[2] = {};
	if (graphicsQueueIndex != computeQueueIndex)
	{
		dqci[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		dqci[0].queueFamilyIndex = graphicsQueueIndex;
		dqci[0].queueCount = 1;
		dqci[0].pQueuePriorities = queuePriorities;

		dqci[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		dqci[1].queueFamilyIndex = computeQueueIndex;
		dqci[1].queueCount = 1;
		dqci[1].pQueuePriorities = queuePriorities;
	}
	else
	{
		dqci[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		dqci[0].queueFamilyIndex = graphicsQueueIndex;
		dqci[0].queueCount = 2;
		dqci[0].pQueuePriorities = queuePriorities;
	}

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.pNext = nullptr;
    dci.queueCreateInfoCount = (graphicsQueueIndex != computeQueueIndex) ? 2 : 1;
    dci.pQueueCreateInfos = dqci;
	dci.enabledLayerCount = (uint32_t)validationLayers.size();
	dci.ppEnabledLayerNames = validationLayers.c_ptr();
    dci.enabledExtensionCount = sizeof_array(c_deviceExtensions);
    dci.ppEnabledExtensionNames = c_deviceExtensions;

#if !defined(__ANDROID__) && !defined(__RPI__)
    VkPhysicalDeviceFeatures features = {};
	features.sampleRateShading = VK_TRUE;
    features.shaderClipDistance = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;
    dci.pEnabledFeatures = &features;
#endif

#if !defined(__ANDROID__) && !defined(__RPI__)
	VkPhysicalDevice8BitStorageFeaturesKHR s8 = {};
	s8.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
	s8.pNext = nullptr;
	s8.storageBuffer8BitAccess = VK_FALSE;
	s8.uniformAndStorageBuffer8BitAccess = VK_TRUE;
	s8.storagePushConstant8 = VK_FALSE;
	dci.pNext = &s8;

	VkPhysicalDevice16BitStorageFeatures s16 = {};
	s16.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
	s16.pNext = nullptr;
	s16.storageBuffer16BitAccess = VK_FALSE;
	s16.uniformAndStorageBuffer16BitAccess = VK_TRUE;
	s16.storagePushConstant16 = VK_FALSE;
	s16.storageInputOutput16 = VK_FALSE;
	s8.pNext = &s16;

	VkPhysicalDeviceFloat16Int8FeaturesKHR f16 = {};
	f16.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
	f16.pNext = nullptr;
	f16.shaderFloat16 = VK_FALSE;
	f16.shaderInt8 = VK_TRUE;
	s16.pNext = &f16;
#endif

    if (vkCreateDevice(m_physicalDevice, &dci, 0, &m_logicalDevice) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create logical device." << Endl;
		return false;
	}

	// Create memory allocator.
	VmaVulkanFunctions vf = {};
	vf.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vf.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vf.vkAllocateMemory = vkAllocateMemory;
	vf.vkFreeMemory = vkFreeMemory;
	vf.vkMapMemory = vkMapMemory;
	vf.vkUnmapMemory = vkUnmapMemory;
	vf.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vf.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vf.vkBindBufferMemory = vkBindBufferMemory;
	vf.vkBindImageMemory = vkBindImageMemory;
	vf.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vf.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vf.vkCreateBuffer = vkCreateBuffer;
	vf.vkDestroyBuffer = vkDestroyBuffer;
	vf.vkCreateImage = vkCreateImage;
	vf.vkDestroyImage = vkDestroyImage;
	vf.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vf.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vf.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;

	VmaAllocatorCreateInfo aci = {};
	aci.physicalDevice = m_physicalDevice;
	aci.device = m_logicalDevice;
	aci.pVulkanFunctions = &vf;
	aci.instance = m_instance;
#if defined(__IOS__)
	aci.preferredLargeHeapBlockSize = 32 * 1024 * 1024;
#endif
	if (vmaCreateAllocator(&aci, &m_allocator) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to create allocator." << Endl;
		return false;
	}

	m_context = new Context(
		m_physicalDevice,
		m_logicalDevice,
		m_allocator,
		graphicsQueueIndex,
		computeQueueIndex
	);

	m_shaderModuleCache = new ShaderModuleCache(m_logicalDevice);
	m_pipelineLayoutCache = new PipelineLayoutCache(m_logicalDevice);
	m_maxAnisotropy = desc.maxAnisotropy;
	m_mipBias = desc.mipBias;

	log::info << L"Vulkan render system created successfully." << Endl;
	return true;
}

void RenderSystemVk::destroy()
{
	m_shaderModuleCache = nullptr;
	m_pipelineLayoutCache = nullptr;
	m_context = nullptr;
	finalizeVulkanApi();
}

bool RenderSystemVk::reset(const RenderSystemDesc& desc)
{
	// \todo Update mipmap bias and maximum anisotropy.
	return true;
}

void RenderSystemVk::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo.dedicatedMemoryTotal = 0;
	outInfo.sharedMemoryTotal = 0;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
}

uint32_t RenderSystemVk::getDisplayModeCount() const
{
#if defined(_WIN32)
	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(nullptr, count, &dmgl))
		++count;

	return count;
#else
	return 0;
#endif
}

DisplayMode RenderSystemVk::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);
	EnumDisplaySettings(nullptr, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#else
	return DisplayMode();
#endif
}

DisplayMode RenderSystemVk::getCurrentDisplayMode() const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#elif defined(__LINUX__) || defined(__RPI__)
	int screen = DefaultScreen(m_display);
	DisplayMode dm;
	dm.width = DisplayWidth(m_display, screen);
	dm.height = DisplayHeight(m_display, screen);
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#elif defined(__ANDROID__)
	DisplayMode dm;
	dm.width = m_screenWidth;
	dm.height = m_screenHeight;
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#elif defined(__IOS__)
	DisplayMode dm;
	dm.width = getScreenWidth();
	dm.height = getScreenHeight();
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#else
	return DisplayMode();
#endif
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 0.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_context,
		m_instance
	);
	if (renderView->create(desc))
		return renderView;
	else
		return nullptr;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_context,
		m_instance
	);
	if (renderView->create(desc))
		return renderView;
	else
		return nullptr;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	VkVertexInputBindingDescription vibd = {};
	vibd.binding = 0;
	vibd.stride = getVertexSize(vertexElements);
	vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	AlignedVector< VkVertexInputAttributeDescription > vads;
	for (auto ve : vertexElements)
	{
		auto& vad = vads.push_back();
		vad.location = VertexAttributes::getLocation(ve.getDataUsage(), ve.getIndex());
		vad.binding = 0;
		vad.format = c_vkVertexElementFormats[ve.getDataType()];
		vad.offset = ve.getOffset();
	}

	// Calculate hash of vertex declaration.
	Adler32 cs;
	cs.begin();
	cs.feed(vertexElements.c_ptr(), vertexElements.size() * sizeof(VertexElement));
	cs.end();

	if (dynamic)
	{
		Ref< VertexBufferDynamicVk > vb = new VertexBufferDynamicVk(m_context, bufferSize, vibd, vads, cs.get(), m_statistics.vertexBuffers);
		if (vb->create(4))
			return vb;
	}
	else
	{
		Ref< VertexBufferStaticVk > vb = new VertexBufferStaticVk(m_context, bufferSize, vibd, vads, cs.get(), m_statistics.vertexBuffers);
		if (vb->create())
			return vb;
	}

	return nullptr;
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	Ref< IndexBufferVk > buffer = new IndexBufferVk(m_context, indexType, bufferSize, m_statistics.indexBuffers);
	if (buffer->create())
		return buffer;
	else
		return nullptr;
}

Ref< StructBuffer > RenderSystemVk::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize, bool dynamic)
{
	if (dynamic)
	{
		Ref< StructBufferDynamicVk > buffer = new StructBufferDynamicVk(m_context, bufferSize, m_statistics.structBuffers);
		if (buffer->create(4))
			return buffer;
	}
	else
	{
		Ref< StructBufferStaticVk > buffer = new StructBufferStaticVk(m_context, bufferSize, m_statistics.structBuffers);
		if (buffer->create())
			return buffer;
	}
	return nullptr;
}

Ref< ISimpleTexture > RenderSystemVk::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< SimpleTextureVk > texture = new SimpleTextureVk(m_context, m_statistics.simpleTextures);
	if (texture->create(desc, tag))
		return texture;
	else
		return nullptr;
}

Ref< ICubeTexture > RenderSystemVk::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< CubeTextureVk > texture = new CubeTextureVk(m_context, desc, m_statistics.cubeTextures);
	if (texture->create(tag))
		return texture;
	else
		return nullptr;
}

Ref< IVolumeTexture > RenderSystemVk::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< VolumeTextureVk > texture = new VolumeTextureVk(m_context, m_statistics.volumeTextures);
	if (texture->create(desc, tag))
		return texture;
	else
		return nullptr;
}

Ref< IRenderTargetSet > RenderSystemVk::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag)
{
	Ref< RenderTargetSetVk > renderTargetSet = new RenderTargetSetVk(m_context, m_statistics.renderTargetSets);
	if (renderTargetSet->create(desc, sharedDepthStencil, tag))
		return renderTargetSet;
	else
		return nullptr;
}

Ref< IProgram > RenderSystemVk::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceVk > resource = dynamic_type_cast< const ProgramResourceVk* >(programResource);
	if (!resource)
		return nullptr;

	Ref< ProgramVk > program = new ProgramVk(m_context, m_statistics.programs);
	if (program->create(m_shaderModuleCache, m_pipelineLayoutCache, resource, m_maxAnisotropy, m_mipBias, tag))
		return program;
	else
		return nullptr;
}

void RenderSystemVk::purge()
{
	if (m_context)
		m_context->savePipelineCache();
}

void RenderSystemVk::getStatistics(RenderSystemStatistics& outStatistics) const
{
	VmaStats stats = {};
	vmaCalculateStats(m_allocator, &stats);

	outStatistics = m_statistics;
	outStatistics.memoryUsage = stats.total.usedBytes;
}

	}
}
