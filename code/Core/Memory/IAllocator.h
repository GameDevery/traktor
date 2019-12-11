#pragma once

#include <cstddef>
#include "Core/IRefCount.h"

namespace traktor
{

/*! Allocator interface.
 * \ingroup Core
 *
 * Allocators used by the Heap must implement this interface.
 */
class IAllocator : public IRefCount
{
public:
	virtual void* alloc(size_t size, size_t align, const char* const tag) = 0;

	virtual void free(void* ptr) = 0;
};

}

