#ifndef traktor_AnsiEncoding_H
#define traktor_AnsiEncoding_H

#include "Core/Io/IEncoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief ANSI encoding.
 * \ingroup Core
 */
class T_DLLCLASS AnsiEncoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	virtual int translate(const wchar_t* chars, int count, uint8_t* out) const;

	virtual int translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const;
};

}

#endif	// traktor_AnsiEncoding_H
