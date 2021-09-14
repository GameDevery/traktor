#pragma once

#include "Core/Rtti/ITypedObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct IObjectRefDebugger;

/*! Managed object base class.
 * \ingroup Core
 */
class T_DLLCLASS Object : public RefCountImpl< ITypedObject >
{
	T_RTTI_CLASS;

public:
	virtual void addRef(void* owner) const override
#if !defined(_DEBUG)
	{
		++m_refCount;
	}
#else
	;
#endif

	virtual void release(void* owner) const override
#if !defined(_DEBUG)
	{
		if (--m_refCount == 0)
			finalRelease();
	}
#else
	;
#endif

	int32_t getReferenceCount() const
#if !defined(_DEBUG)
	{
		return m_refCount;
	}
#else
	;
#endif

	void* operator new (size_t size);

	void* operator new (size_t size, void* memory);

	void operator delete (void* ptr);

	void operator delete (void* ptr, void* memory);

	static void setReferenceDebugger(IObjectRefDebugger* refDebugger);

	static int32_t getHeapObjectCount();

private:
#if defined(_DEBUG)
	static IObjectRefDebugger* ms_refDebugger;
#endif

	void finalRelease() const;
};

}
