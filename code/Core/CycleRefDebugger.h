#ifndef traktor_CycleRefDebugger_H
#define traktor_CycleRefDebugger_H

#include <map>
#include <vector>
#include "Core/IObjectRefDebugger.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Check for leaking objects from circular references.
 * \ingroup Core
 */
class T_DLLCLASS CycleRefDebugger : public IObjectRefDebugger
{
public:
	CycleRefDebugger();

	virtual void addObject(void* object, size_t size);

	virtual void removeObject(void* object);

	virtual void addObjectRef(void* ref, void* object);

	virtual void removeObjectRef(void* ref, void* object);

private:
	struct ObjInfo
	{
		size_t size;
		size_t nrefs;										//< Number of references to this object.
		size_t ntrace;										//< Number of trace references to this object.
		std::vector< std::pair< void*, ObjInfo* > > refs;	//< Objects referenced from this object.
		uint32_t tag;
	};

	Semaphore m_lock;
	std::map< void*, ObjInfo > m_objects;
	uint32_t m_tag;

	void trace(ObjInfo* object);
};

}

#endif	// traktor_CycleRefDebugger_H
