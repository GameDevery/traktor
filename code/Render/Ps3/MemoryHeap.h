#ifndef traktor_render_LocalMemoryManager_H
#define traktor_render_LocalMemoryManager_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace render
	{

class MemoryHeapObject;

class MemoryHeap : public Object
{
public:
	MemoryHeap(void* heap, size_t heapSize, uint8_t location);

	MemoryHeapObject* alloc(size_t size, size_t align, bool immutable);

	size_t getAvailable() const;

	size_t getObjectCount() const;

	void compact();

private:
	friend class MemoryHeapObject;

	mutable Semaphore m_lock;
	uint8_t* m_heap;
	size_t m_heapSize;
	uint8_t m_location;
	std::vector< MemoryHeapObject* > m_objects;		//< Sorted list of alive objects.
	bool m_shouldCompact;
	uint32_t m_waitLabel;

	void free(MemoryHeapObject* object);
};

	}
}

#endif	// traktor_render_LocalMemoryManager_H
