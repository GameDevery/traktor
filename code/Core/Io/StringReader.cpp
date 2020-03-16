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
		int64_t result = m_stream->read(&m_buffer[m_count], sizeof(m_buffer) - m_count);
		if (result > 0)
			m_count += result;
		else if (m_count <= 0)
			return 0;
	}

	T_ASSERT(m_count > 0);

	int32_t result = m_encoding->translate(m_buffer, m_count, ch);
	if (result <= 0)
		return 0;

	std::memmove(&m_buffer[0], &m_buffer[result], m_count - result);
	m_count -= result;

	return ch;
}

int64_t StringReader::readLine(std::wstring& out)
{
	wchar_t ch;

	m_line.reserve(200);
	m_line.resize(0);

	for (;;)
	{
		if (m_count < sizeof(m_buffer))
		{
			int64_t result = -1;
			if (m_stream)
			{
				result = m_stream->read(&m_buffer[m_count], sizeof(m_buffer) - m_count);
				if (result < 0)
					m_stream = nullptr;
			}
			if (result > 0)
				m_count += result;
			else if (m_count <= 0 && m_line.empty())
			{
				out.clear();
				return -1;
			}
		}

		if (m_count <= 0)
			break;

		int32_t result = m_encoding->translate(m_buffer, m_count, ch);
		if (result <= 0)
		{
			// Need more characters in buffer; read another byte.
			if (m_stream != nullptr && m_count < sizeof(m_buffer))
				continue;
			else
				return -1;
		}

		std::memmove(&m_buffer[0], &m_buffer[result], m_count - result);
		m_count -= result;

		if (ch == L'\n')
			break;
		else if (ch != L'\r')
			m_line.push_back(ch);
	}

	out = std::wstring(m_line.begin(), m_line.end());
	return (int64_t)out.length();
}

}
