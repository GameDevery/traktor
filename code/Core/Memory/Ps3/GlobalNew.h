#ifndef traktor_GlobalNew_H
#define traktor_GlobalNew_H

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"

/*! \brief Global new/delete operators.
 * \ingroup Core
 *
 * These are overloaded in order to make PS3
 * small allocations more efficient.
 */
//@{

void* operator new (size_t size)
{
	return traktor::getAllocator()->alloc(size, 4, "::new");
}

void* operator new (size_t size, size_t align)
{
	return traktor::getAllocator()->alloc(size, align, "::new+");
}

void operator delete (void* ptr)
{
	if (ptr)
		traktor::getAllocator()->free(ptr);
}

void* operator new[] (size_t size)
{
	return traktor::getAllocator()->alloc(size, 4, "::new[]");
}

void* operator new[] (size_t size, size_t align)
{
	return traktor::getAllocator()->alloc(size, align, "::new[]+");
}

void operator delete[] (void* ptr)
{
	if (ptr)
		traktor::getAllocator()->free(ptr);
}

//@}

#endif	// traktor_GlobalNew_H
