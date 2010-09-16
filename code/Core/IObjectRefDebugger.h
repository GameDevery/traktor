#ifndef traktor_IObjectRefDebugger_H
#define traktor_IObjectRefDebugger_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Managed object reference debugger interface.
 * \ingroup Core
 */
struct T_NOVTABLE IObjectRefDebugger
{
	virtual void addObject(void* object, size_t size) = 0;

	virtual void removeObject(void* object) = 0;

	virtual void addObjectRef(void* ref, void* object) = 0;

	virtual void removeObjectRef(void* ref, void* object) = 0;
};

}

#endif	// traktor_IObjectRefDebugger_H
