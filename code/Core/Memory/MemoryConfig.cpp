#include "Core/Memory/Alloc.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Memory/TrackAllocator.h"

namespace traktor
{
	namespace
	{

IAllocator* s_stdAllocator = 0;
IAllocator* s_allocator = 0;

#if !defined(_PS3)
void destroyAllocator()
{
	if (s_allocator != s_stdAllocator)
		freeDestruct(s_allocator);

	freeDestruct(s_stdAllocator);

	s_stdAllocator = 0;
	s_allocator = 0;
}
#endif

	}

IAllocator* getAllocator()
{
	if (!s_allocator)
	{
		s_stdAllocator = allocConstruct< StdAllocator >();
		s_stdAllocator->addRef(0);

#if !defined(_DEBUG)
		s_allocator = allocConstruct< FastAllocator >(s_stdAllocator);
#elif defined(_PS3)
		s_allocator = s_stdAllocator;
#else
		s_allocator = allocConstruct< TrackAllocator >(s_stdAllocator);
#endif

		s_allocator->addRef(0);

#if !defined(__GNUC__) && !defined(_PS3)
		std::atexit(destroyAllocator);
#endif
	}
	return s_allocator;
}

}
