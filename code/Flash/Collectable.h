#pragma once

#include "Core/Object.h"
#include "Core/Containers/IntrusiveList.h"
#include "Core/Containers/SmallSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Base of garbage collectable classes.
 * \ingroup Flash
 */
class T_DLLCLASS Collectable : public Object
{
	T_RTTI_CLASS;

public:
	class IVisitor
	{
	public:
		virtual ~IVisitor() {}

		virtual void operator () (Collectable* childCollectable) const = 0;
	};

	class IWeakRefDispose
	{
	public:
		virtual ~IWeakRefDispose() {}

		virtual void disposeReference(Collectable* collectable) = 0;
	};

	Collectable();

	virtual ~Collectable();

	virtual void addRef(void* owner) const override final;

	virtual void release(void* owner) const override final;

	/*! \brief Register a weak reference. */
	void addWeakRef(IWeakRefDispose* weakRefDispose);

	/*! \brief Remove a weak reference. */
	void releaseWeakRef(IWeakRefDispose* weakRefDispose);

	/*! \brief Get alive collectible instance count. */
	static int32_t getInstanceCount();

protected:
	typedef void (*visitor_t)(Collectable* memberObject);

	virtual void trace(/*const IVisitor& visitor*/ visitor_t visitor) const = 0;

	virtual void dereference() = 0;

private:
	friend struct DefaultLink< Collectable >;
	friend class GC;

	enum TraceColor
	{
		TcBlack = 0,
		TcPurple = 1,
		TcGray = 2,
		TcWhite = 3
	};

	static int32_t ms_instanceCount;
	Collectable* m_prev;	//!< Intrusive list chain members.
	Collectable* m_next;
	mutable SmallSet< IWeakRefDispose* >* m_weakRefDisposes;
	mutable int32_t m_traceRefCount;
	mutable uint8_t m_traceColor;
	mutable bool m_traceBuffered;

	void traceMarkGray();

	void traceScan();

	void traceScanBlack();

	void traceCollectWhite();

	static void visitorMarkGray(Collectable* memberObject);

	static void visitorScan(Collectable* memberObject);

	static void visitorScanBlack(Collectable* memberObject);
};

	}
}

