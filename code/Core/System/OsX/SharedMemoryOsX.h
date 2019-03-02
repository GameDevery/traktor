#pragma once

#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryOsX : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryOsX(uint32_t size);

	virtual ~SharedMemoryOsX();

	virtual Ref< IStream > read(bool exclusive) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > write() T_OVERRIDE T_FINAL;

	virtual bool clear() T_OVERRIDE T_FINAL;

private:
	uint32_t m_size;
	void* m_buffer;
};

}

