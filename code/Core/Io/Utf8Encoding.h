#pragma once

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

/*! \brief UTF-8 encoding.
 * \ingroup Core
 */
class T_DLLCLASS Utf8Encoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	Utf8Encoding() {}

	virtual int translate(const wchar_t* chars, int count, uint8_t* out) const override final;

	virtual int translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const override final;
};

}

