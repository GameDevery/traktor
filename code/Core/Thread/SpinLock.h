#pragma once

#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Blocking spin lock primitive.
 * \ingroup Core
 */
class T_DLLCLASS SpinLock : public IWaitable
{
public:
	SpinLock();

	virtual ~SpinLock();

	virtual bool wait(int32_t timeout = -1);

	void release();

private:
	int32_t m_lock;
};

}

