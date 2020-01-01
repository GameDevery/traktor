#include <algorithm>
#include "Core/Memory/Alloc.h"
#include "Render/Context/RenderContext.h"

#if defined(_DEBUG)
#	include "Core/Log/Log.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

const float c_distanceQuantizeRangeInv = 1.0f / 10.0f;

T_FORCE_INLINE bool SortOpaquePredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
// Don't sort front-to-back on iOS as it's a TDBR architecture thus
// we focus on minimizing state changes on the CPU instead.
#if !defined(__IOS__)
	float d1 = std::floor(renderBlock1->distance * c_distanceQuantizeRangeInv);
	float d2 = std::floor(renderBlock2->distance * c_distanceQuantizeRangeInv);

	if (d1 < d2)
		return true;
	else if (d1 > d2)
		return false;
#endif

	return renderBlock1->program < renderBlock2->program;
}

T_FORCE_INLINE bool SortAlphaBlendPredicate(const RenderBlock* renderBlock1, const RenderBlock* renderBlock2)
{
	return renderBlock1->distance > renderBlock2->distance;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderContext", RenderContext, Object)

RenderContext::RenderContext(uint32_t heapSize)
:	m_heapEnd(0)
,	m_heapPtr(0)
{
	m_heap.reset(static_cast< uint8_t* >(Alloc::acquireAlign(heapSize, 16, T_FILE_LINE)));
	T_FATAL_ASSERT_M(m_heap.ptr(), L"Out of memory (Render context)");
	m_heapEnd = m_heap.ptr() + heapSize;
	m_heapPtr = m_heap.ptr();
}

RenderContext::~RenderContext()
{
	m_heap.release();
}

void* RenderContext::alloc(uint32_t blockSize)
{
	if (m_heapPtr + blockSize >= m_heapEnd)
		T_FATAL_ERROR;

	void* ptr = m_heapPtr;
	m_heapPtr += blockSize;

	return ptr;
}

void* RenderContext::alloc(uint32_t blockSize, uint32_t align)
{
	T_ASSERT(align > 0);
	m_heapPtr = alignUp(m_heapPtr, align);
	return alloc(blockSize);
}

void RenderContext::enqueue(RenderBlock* renderBlock)
{
	m_renderQueue.push_back(renderBlock);
}

void RenderContext::draw(uint32_t type, RenderBlock* renderBlock)
{
	if (type == RpSetup)
		m_priorityQueue[0].push_back(renderBlock);
	else if (type == RpOpaque)
		m_priorityQueue[1].push_back(renderBlock);
	else if (type == RpPostOpaque)
		m_priorityQueue[2].push_back(renderBlock);
	else if (type == RpAlphaBlend)
		m_priorityQueue[3].push_back(renderBlock);
	else if (type == RpPostAlphaBlend)
		m_priorityQueue[4].push_back(renderBlock);
	else if (type == RpOverlay)
		m_priorityQueue[5].push_back(renderBlock);
}

void RenderContext::merge(uint32_t priorities)
{
	// Merge setup blocks unsorted.
	if (priorities & RpSetup)
	{
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[0].begin(), m_priorityQueue[0].end());
		m_priorityQueue[0].resize(0);
	}

	// Merge opaque blocks, sorted by shader.
	if (priorities & RpOpaque)
	{
		std::sort(m_priorityQueue[1].begin(), m_priorityQueue[1].end(), SortOpaquePredicate);
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[1].begin(), m_priorityQueue[1].end());
		m_priorityQueue[1].resize(0);
	}

	// Merge post opaque blocks, sorted by shader.
	if (priorities & RpPostOpaque)
	{
		std::sort(m_priorityQueue[2].begin(), m_priorityQueue[2].end(), SortOpaquePredicate);
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[2].begin(), m_priorityQueue[2].end());
		m_priorityQueue[2].resize(0);
	}

	// Merge alpha blend blocks back to front.
	if (priorities & RpAlphaBlend)
	{
		std::sort(m_priorityQueue[3].begin(), m_priorityQueue[3].end(), SortAlphaBlendPredicate);
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[3].begin(), m_priorityQueue[3].end());
		m_priorityQueue[3].resize(0);
	}

	// Merge post alpha blend blocks back to front.
	if (priorities & RpPostAlphaBlend)
	{
		std::sort(m_priorityQueue[4].begin(), m_priorityQueue[4].end(), SortAlphaBlendPredicate);
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[4].begin(), m_priorityQueue[4].end());
		m_priorityQueue[4].resize(0);
	}

	// Merge overlay blocks unsorted.
	if (priorities & RpOverlay)
	{
		m_renderQueue.insert(m_renderQueue.end(), m_priorityQueue[5].begin(), m_priorityQueue[5].end());
		m_priorityQueue[5].resize(0);
	}
}

void RenderContext::render(IRenderView* renderView) const
{
	for (auto renderBlock : m_renderQueue)
		renderBlock->render(renderView);
}

void RenderContext::flush()
{
	// Reset queues and heap.
	for (int32_t i = 0; i < sizeof_array(m_priorityQueue); ++i)
	{
#if defined(_DEBUG)
		if (!m_priorityQueue[i].empty())
		{
			const wchar_t* c_queueNames[] = { L"Setup", L"Opaque", L"Post-Opaque", L"Alpha-Blend", L"Post-Alpha-Blend", L"Overlay" };
			log::debug << L"Still " << (int32_t)m_priorityQueue[i].size() << L" non-merged render blocks in priority queue \"" << c_queueNames[i] << L"\", discarded." << Endl;
		}
#endif
		// As blocks are allocated from a fixed pool we need to manually call destructors.
		for (auto renderBlock : m_priorityQueue[i])
			renderBlock->~RenderBlock();

		m_priorityQueue[i].resize(0);
	}

	// As blocks are allocated from a fixed pool we need to manually call destructors.
	for (auto renderBlock : m_renderQueue)
		renderBlock->~RenderBlock();

	m_renderQueue.resize(0);

	m_heapPtr = m_heap.ptr();
}

bool RenderContext::havePendingDraws() const
{
	for (int32_t i = 0; i < sizeof_array(m_priorityQueue); ++i)
	{
		if (!m_priorityQueue[i].empty())
			return true;
	}
	return false;
}

	}
}
