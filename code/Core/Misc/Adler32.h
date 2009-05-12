#ifndef traktor_Adler32_H
#define traktor_Adler32_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/* \brief Adler32 checksum.
 * \ingroup Core
 */
class T_DLLCLASS Adler32
{
public:
	Adler32();

	void begin();

	void feed(const void* buffer, uint32_t bufferSize);

	void end();

	const uint32_t get() const;

private:
	uint32_t m_A;
	uint32_t m_B;
	uint32_t m_feed;
};

}

#endif	// traktor_Adler32_H
