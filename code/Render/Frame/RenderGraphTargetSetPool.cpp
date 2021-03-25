#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const int32_t c_maxUnusuedFrames = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphTargetSetPool", RenderGraphTargetSetPool, Object)

RenderGraphTargetSetPool::RenderGraphTargetSetPool(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

void RenderGraphTargetSetPool::destroy()
{
	m_renderSystem = nullptr;
	for (auto& pool : m_pool)
	{
		T_FATAL_ASSERT(pool.acquired.empty());
		for (auto& target : pool.free)
			safeDestroy(target.rts);
	}
	m_pool.clear();
}

IRenderTargetSet* RenderGraphTargetSetPool::acquire(
	const wchar_t* name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	int32_t referenceWidth,
	int32_t referenceHeight,
	uint32_t multiSample,
	handle_t persistentHandle
)
{
	// Create descriptor for given reference size.
	RenderTargetSetCreateDesc rtscd;
	rtscd.count = targetSetDesc.count;
	rtscd.width = targetSetDesc.width;
	rtscd.height = targetSetDesc.height;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = targetSetDesc.createDepthStencil;
	rtscd.usingPrimaryDepthStencil = targetSetDesc.usingPrimaryDepthStencil;
	rtscd.usingDepthStencilAsTexture = targetSetDesc.usingDepthStencilAsTexture;
	rtscd.ignoreStencil = targetSetDesc.usingDepthStencilAsTexture;	// Cannot have stencil on read-back depth targets.
	rtscd.generateMips = targetSetDesc.generateMips;

	if (targetSetDesc.usingPrimaryDepthStencil)
		rtscd.multiSample = multiSample;

	for (int32_t i = 0; i < targetSetDesc.count; ++i)
		rtscd.targets[i].format = targetSetDesc.targets[i].colorFormat;
		
	if (targetSetDesc.referenceWidthDenom > 0)
		rtscd.width = (referenceWidth * targetSetDesc.referenceWidthMul + targetSetDesc.referenceWidthDenom - 1) / targetSetDesc.referenceWidthDenom;
	if (targetSetDesc.referenceHeightDenom > 0)
		rtscd.height = (referenceHeight * targetSetDesc.referenceHeightMul + targetSetDesc.referenceHeightDenom - 1) / targetSetDesc.referenceHeightDenom;

	if (targetSetDesc.maxWidth > 0)
		rtscd.width = min< int32_t >(rtscd.width, targetSetDesc.maxWidth);
	if (targetSetDesc.maxHeight > 0)
		rtscd.height = min< int32_t >(rtscd.height, targetSetDesc.maxHeight);

	// Find pool matching target description.
	auto it = std::find_if(
        m_pool.begin(),
        m_pool.end(),
        [&](const RenderGraphTargetSetPool::Pool& p)
        {
            if (p.sharedDepthStencilTargetSet != sharedDepthStencilTargetSet)
                return false;

			if (p.persistentHandle != persistentHandle)
				return false;

            if (
				p.rtscd.count != rtscd.count ||
				p.rtscd.width != rtscd.width ||
				p.rtscd.height != rtscd.height ||
				p.rtscd.multiSample != rtscd.multiSample ||
				p.rtscd.createDepthStencil != rtscd.createDepthStencil ||
				p.rtscd.usingDepthStencilAsTexture != rtscd.usingDepthStencilAsTexture ||
				p.rtscd.usingPrimaryDepthStencil != rtscd.usingPrimaryDepthStencil ||
				p.rtscd.ignoreStencil != rtscd.ignoreStencil ||
				p.rtscd.generateMips != rtscd.generateMips
            )
                return false;

 			for (int32_t i = 0; i < p.rtscd.count; ++i)
			{
				if (
					p.rtscd.targets[i].format != rtscd.targets[i].format ||
					p.rtscd.targets[i].sRGB != rtscd.targets[i].sRGB
				)
					return false;
			}

            return true;
        }
    );

	// Get or create pool.
	Pool* pool = nullptr;
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		pool = &m_pool.push_back();
		pool->rtscd = rtscd;
        pool->sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free target, if no one left we need to create a new target.
	if (!pool->free.empty())
	{
		Target target = pool->free.back();

		pool->free.pop_back();
		pool->acquired.push_back(target.rts);

		target.rts->setDebugName(name);
		return target.rts;
	}
	else
	{
		if (sharedDepthStencilTargetSet)
		{
			int32_t sharedWidth = sharedDepthStencilTargetSet->getWidth();
			int32_t sharedHeight = sharedDepthStencilTargetSet->getHeight();
			T_FATAL_ASSERT(sharedWidth == rtscd.width);
			T_FATAL_ASSERT(sharedHeight == rtscd.height);
		}

		Ref< IRenderTargetSet > rts = m_renderSystem->createRenderTargetSet(rtscd, sharedDepthStencilTargetSet, T_FILE_LINE_W);
		if (rts)
		{
			rts->setDebugName(name);
			pool->acquired.push_back(rts);
		}
		return rts;
	}
}

void RenderGraphTargetSetPool::release(IRenderTargetSet* targetSet)
{
	T_ANONYMOUS_VAR(Ref< IRenderTargetSet >)(targetSet);
	for (auto& pool : m_pool)
	{
		auto it = std::remove_if(pool.acquired.begin(), pool.acquired.end(), [&](const IRenderTargetSet* rts) {
			return rts == targetSet;
		});
		if (it != pool.acquired.end())
		{
			pool.acquired.erase(it, pool.acquired.end());
			pool.free.push_back({ targetSet, 0 });
			break;
		}
	}
}

void RenderGraphTargetSetPool::cleanup()
{
	int32_t freed = 0;
	for (auto& pool : m_pool)
	{
		T_FATAL_ASSERT(pool.acquired.empty());
		auto it = std::remove_if(pool.free.begin(), pool.free.end(), [](const Target& target) {
			return target.unused > c_maxUnusuedFrames;
		});
		if (it != pool.free.end())
		{
			for (auto it2 = it; it2 != pool.free.end(); ++it2)
			{
				if (it2->rts)
					it2->rts->destroy();
				++freed;
			}
			pool.free.erase(it, pool.free.end());
		}
		for (auto& target : pool.free)
			target.unused++;
	}
	if (freed > 0)
	{
		auto it = std::remove_if(m_pool.begin(), m_pool.end(), [](const Pool& pool) {
			return pool.free.empty() && pool.acquired.empty();
		});
		m_pool.erase(it, m_pool.end());
	}
}

	}
}
