#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Io/StringReader.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StringReader", StringReader, Object)

StringReader::StringReader(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_count(0)
{
}

wchar_t StringReader::readChar()
{
	wchar_t ch;

	if (m_count < sizeof(m_buffer))
	{
		int result = m_stream->read(&m_buffer[m_count], sizeof(m_buffer) - m_count);
		if (result > 0)
			m_count += result;
		else if (m_count <= 0)
			return 0;
	}

	T_ASSERT (m_count > 0);

	int result = m_encoding->translate(m_buffer, m_count, ch);
	if (result <= 0)
		return 0;

	std::memmove(&m_buffer[0], &m_buffer[result], m_count - result);
	m_count -= result;

	return ch;
}

int StringReader::readLine(std::wstring& out)
{
	wchar_t ch;

	out = L"";

	for (;;)
	{
		if (m_count < sizeof(m_buffer))
		{
			int result = m_stream->read(&m_buffer[m_count], sizeof(m_buffer) - m_count);
			if (result > 0)
				m_count += result;
			else if (m_count <= 0 && out.empty())
				return -1;
		}
		
		if (m_count <= 0)
			break;

		int result = m_encoding->translate(m_buffer, m_count, ch);
		if (result <= 0)
			break;

		std::memmove(&m_buffer[0], &m_buffer[result], m_count - result);
		m_count -= result;

		if (ch == L'\n')
			break;
		else if (ch != L'\r')
			out += ch;
	}

	return int(out.length());
}

}
