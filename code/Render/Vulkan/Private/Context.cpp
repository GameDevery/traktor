
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/UniformBufferPool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Context", Context, Object)

Context::Context(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	uint32_t graphicsQueueIndex
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_pipelineCache(0)
,	m_descriptorPool(0)
,	m_views(0)
,	m_descriptorPoolRevision(0)
{
	AlignedVector< uint8_t > buffer;

	// Create queues.
	m_graphicsQueue = Queue::create(this, graphicsQueueIndex);

	// Create pipeline cache.
	VkPipelineCacheCreateInfo pcci = {};
	pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pcci.flags = 0;
	pcci.initialDataSize = 0;
	pcci.pInitialData = nullptr;

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (file)
	{
	 	uint32_t size = (uint32_t)file->available();
		buffer.resize(size);
	 	file->read(buffer.ptr(), size);
	 	file->close();

		pcci.initialDataSize = size;
		pcci.pInitialData = buffer.c_ptr();

		log::debug << L"Pipeline cache \"" << ss.str() << L"\" loaded succesfully." << Endl;
	}
	else
		log::debug << L"No pipeline cache found; creating new cache." << Endl;


	vkCreatePipelineCache(
		m_logicalDevice,
		&pcci,
		nullptr,
		&m_pipelineCache
	);

	// Create descriptor set pool.
	VkDescriptorPoolSize dps[4];
	dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	dps[0].descriptorCount = 40000;
	dps[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	dps[1].descriptorCount = 40000;
	dps[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	dps[2].descriptorCount = 40000;
	dps[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	dps[3].descriptorCount = 4000;

	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	dpci.maxSets = 16000;
	dpci.poolSizeCount = sizeof_array(dps);
	dpci.pPoolSizes = dps;
	vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &m_descriptorPool);

	// Create uniform buffer pools.
	m_uniformBufferPools[0] = new UniformBufferPool(this,   1000, L"Once");
	m_uniformBufferPools[1] = new UniformBufferPool(this,  10000, L"Frame");
	m_uniformBufferPools[2] = new UniformBufferPool(this, 100000, L"Draw");
}

Context::~Context()
{
	// Destroy uniform buffer pools.
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
	{
		m_uniformBufferPools[i]->destroy();
		m_uniformBufferPools[i] = nullptr;
	}

	// Destroy descriptor pool.
	if (m_descriptorPool != 0)
	{
		vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
		m_descriptorPool = 0;
	}
}

void Context::incrementViews()
{
	Atomic::increment(m_views);
}

void Context::decrementViews()
{
	Atomic::decrement(m_views);
}

void Context::addDeferredCleanup(const cleanup_fn_t& fn)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

	// In case there are no render views which can perform cleanup
	// after each frame, we do this immediately.
	if (m_views > 0)
		m_cleanupFns.push_back(fn);
	else
	{
		for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
			m_uniformBufferPools[i]->flush();

		fn(this);
	}
}

bool Context::needCleanup() const
{
	return !m_cleanupFns.empty();
}

void Context::performCleanup()
{
	if (m_cleanupFns.empty())
		return;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

		// Wait until GPU is idle to ensure resources are not used, or pending, in some queue before destroying them.
		vkDeviceWaitIdle(m_logicalDevice);

		while (!m_cleanupFns.empty())
		{
			// Take over vector in case more resources are added for cleanup from callbacks.
			AlignedVector< cleanup_fn_t > cleanupFns;
			cleanupFns.swap(m_cleanupFns);

			// Invoke cleanups.
			for (const auto& cleanupFn : cleanupFns)
				cleanupFn(this);
		}
		
		// Reset descriptor pool since we need to ensure programs clear their cached descriptor sets.
		vkResetDescriptorPool(m_logicalDevice, m_descriptorPool, 0);
		m_descriptorPoolRevision++;
	}
}

void Context::recycle()
{
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
		m_uniformBufferPools[i]->recycle();
}

bool Context::savePipelineCache()
{
	size_t size = 0;
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, nullptr);
	if (!size)
		return true;

	AlignedVector< uint8_t > buffer(size, 0);
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, buffer.ptr());

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	FileSystem::getInstance().makeAllDirectories(Path(ss.str()).getPathOnly());

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to save pipeline cache; failed to create file \"" << ss.str() << L"\"." << Endl;
		return false;
	}

	file->write(buffer.c_ptr(), size);
	file->close();
	
	log::debug << L"Pipeline cache \"" << ss.str() << L"\" saved successfully." << Endl;
	return true;
}

	}
}
