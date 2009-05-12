#ifndef traktor_Utf16Encoding_H
#define traktor_Utf16Encoding_H

#include "Core/Io/Encoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief UTF-16 encoding.
 * \ingroup Core
 */
class T_DLLCLASS Utf16Encoding : public Encoding
{
	T_RTTI_CLASS(Utf16Encoding)

public:
	virtual int translate(const wchar_t* chars, int count, uint8_t* out) const;

	virtual int translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const;
};

}

#endif	// traktor_Utf16Encoding_H
