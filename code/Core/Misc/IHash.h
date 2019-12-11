#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Hash function interface.
 * \ingroup Core
 */
class T_DLLCLASS IHash : public Object
{
	T_RTTI_CLASS;

public:
	/*! Begin feeding data for hash calculation. */
	virtual void begin() = 0;

	/*! Feed data to hash calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feed(const void* buffer, uint64_t bufferSize) = 0;

	/*! End feeding data for hash calculation. */
	virtual void end() = 0;
};

}

