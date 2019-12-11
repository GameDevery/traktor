#pragma once

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"

/*! Global new/delete operators.
 * \ingroup Core
 *
 * These are overloaded in order to make PS4
 * small allocations more efficient.
 */
//@{

void* operator new (size_t size)
{
	return traktor::getAllocator()->alloc(size, 16, "::new");
}

void* operator new (size_t size, size_t align)
{
	return traktor::getAllocator()->alloc(size, align, "::new+");
}

void operator delete (void* ptr) throw ()
{
	if (ptr)
		traktor::getAllocator()->free(ptr);
}

void* operator new[] (size_t size)
{
	return traktor::getAllocator()->alloc(size, 16, "::new[]");
}

void* operator new[] (size_t size, size_t align)
{
	return traktor::getAllocator()->alloc(size, align, "::new[]+");
}

void operator delete[] (void* ptr) throw ()
{
	if (ptr)
		traktor::getAllocator()->free(ptr);
}

//@}

