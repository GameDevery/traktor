#include "Core/Memory/Alloc.h"
#include "Core/Memory/DebugAllocator.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Memory/SystemConstruct.h"
#include "Core/Memory/TrackAllocator.h"

namespace traktor
{
	namespace
	{

IAllocator* s_stdAllocator = nullptr;
IAllocator* s_allocator = nullptr;

#if !defined(_PS3)
void destroyAllocator()
{
	if (s_allocator != s_stdAllocator)
		freeDestruct(s_allocator);

	freeDestruct(s_stdAllocator);

	s_stdAllocator = nullptr;
	s_allocator = nullptr;
}
#endif

	}

IAllocator* getAllocator()
{
	if (!s_allocator)
	{
		s_stdAllocator = allocConstruct< StdAllocator >();
		s_stdAllocator->addRef(nullptr);

#if defined(__IOS__) || defined(__ANDROID__) || defined(_PS3) || defined(__EMSCRIPTEN__) || defined(__APPLE__)
		s_allocator = s_stdAllocator;
#elif !defined(_DEBUG)
		s_allocator = allocConstruct< FastAllocator >(s_stdAllocator);
#else
		//s_allocator = allocConstruct< TrackAllocator >(s_stdAllocator);
		//s_allocator = allocConstruct< DebugAllocator >(s_stdAllocator);
		s_allocator = s_stdAllocator;
#endif

		s_allocator->addRef(nullptr);

#if !defined(_PS3) && !defined(__IOS__)
		std::atexit(destroyAllocator);
#endif
	}
	return s_allocator;
}

}
