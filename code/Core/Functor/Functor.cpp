#include "Core/Functor/Functor.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/JobManager.h"

namespace traktor
{
	namespace
	{

class FunctorHeap : public ISingleton
{
public:
	static FunctorHeap& getInstance()
	{
		static FunctorHeap* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new FunctorHeap();
			SingletonManager::getInstance().addAfter(s_instance, &JobManager::getInstance());
		}
		return *s_instance;
	}

	void* alloc(uint32_t size)
	{
		T_ASSERT_M (size <= MaxFunctorSize, L"Allocation size too big");
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);
		void* ptr = m_blockAllocator.alloc();
		if (!ptr)
			T_FATAL_ERROR;
#if defined(_DEBUG)
		m_count++;
#endif
		return ptr;
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);
		bool result = m_blockAllocator.free(ptr);
		T_ASSERT_M (result, L"Invalid pointer");
#if defined(_DEBUG)
		m_count--;
#endif
	}

protected:
	virtual void destroy() { delete this; }

private:
#if !defined(WINCE)
	enum { MaxFunctorCount = 4096 };
#else
	enum { MaxFunctorCount = 1024 };
#endif
	enum { MaxFunctorSize = 128 };

	void* m_block;
	BlockAllocator m_blockAllocator;
	Semaphore m_allocatorLock;
#if defined(_DEBUG)
	int32_t m_count;
#endif

	FunctorHeap()
	:	m_block(Alloc::acquireAlign(MaxFunctorCount * MaxFunctorSize, 16, T_FILE_LINE))
	,	m_blockAllocator(m_block, MaxFunctorCount, MaxFunctorSize)
#if defined(_DEBUG)
	,	m_count(0)
#endif
	{
	}

	virtual ~FunctorHeap()
	{
		T_ASSERT_M (m_count == 0, L"There are still functors allocated, memory leak?");
		Alloc::freeAlign(m_block);
	}
};

	}

void* Functor::operator new (size_t size)
{
	return FunctorHeap::getInstance().alloc(uint32_t(size));
}

void Functor::operator delete (void* ptr)
{
	FunctorHeap::getInstance().free(ptr);
}

}
