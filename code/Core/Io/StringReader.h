#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
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

class IStream;

/*! Read strings from stream.
 * \ingroup Core
 */
class T_DLLCLASS StringReader : public Object
{
	T_RTTI_CLASS;

public:
	StringReader(IStream* stream, IEncoding* encoding);

	/*! Read character from stream. */
	wchar_t readChar();

	/*! Read string from stream until end-of-line or end-of-file. */
	int64_t readLine(std::wstring& out);

private:
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	AlignedVector< wchar_t > m_line;
	uint8_t m_buffer[IEncoding::MaxEncodingSize];
	int64_t m_count;
};

}

