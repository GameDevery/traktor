#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/Context/RenderBlock.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;

/*! Deferred render context.
 * \ingroup Render
 *
 * A render context is used to defer rendering in a
 * multi-threaded renderer.
 */
class T_DLLCLASS RenderContext : public Object
{
	T_RTTI_CLASS;

public:
	RenderContext(uint32_t heapSize);

	virtual ~RenderContext();

	/*! Allocate a unaligned block of memory from context's heap. */
	void* alloc(uint32_t blockSize);

	/*! Allocate a aligned block of memory from context's heap. */
	void* alloc(uint32_t blockSize, uint32_t align);

	/*! Allocate object from context's heap.
	 *
	 * \note Object's destructor won't be called when
	 * heap is flushed.
	 */
	template < typename ObjectType >
	ObjectType* alloc()
	{
		void* object = alloc((uint32_t)sizeof(ObjectType), (uint32_t)alignOf< ObjectType >());
		return new (object) ObjectType();
	}

	/*! Allocate named object from context's heap. */
	template < typename ObjectType >
	ObjectType* alloc(const char* const name)
	{
		ObjectType* object = alloc< ObjectType >();
#if defined(_DEBUG)
		object->name = name;
#endif
		return object;
	}

	/*! Enqueue a render block in context. */
	void enqueue(RenderBlock* renderBlock);

	/*! Add render block to sorting queue. */
	void draw(uint32_t type, RenderBlock* renderBlock);

	/*! Merge sorting queues into render queue. */
	void merge(uint32_t priorities);

	/*! Render blocks queued in render queue. */
	void render(IRenderView* renderView) const;

	/*! Flush blocks. */
	void flush();

	/*! Check if any draws is pending for merge. */
	bool havePendingDraws() const;

	uint32_t getAllocatedSize() const { return uint32_t(m_heapPtr - m_heap.c_ptr()); }

private:
	AutoPtr< uint8_t, AllocFreeAlign > m_heap;
	uint8_t* m_heapEnd;
	uint8_t* m_heapPtr;
	AlignedVector< RenderBlock* > m_renderQueue;
	AlignedVector< RenderBlock* > m_priorityQueue[6];
};

	}
}

