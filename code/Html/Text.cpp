#include <limits>
#include "Core/Io/OutputStream.h"
#include "Core/Misc/String.h"
#include "Html/Text.h"

namespace traktor
{
	namespace html
	{
		namespace
		{

std::wstring decodeCharacterEntities(const std::wstring& text)
{
	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		size_t nccs = text.find(L'&', offset);
		if (nccs == text.npos)
			break;

		size_t ncce = text.find(L';', nccs);
		if (ncce == text.npos)
			break;

		ss << text.substr(offset, nccs - offset);

		std::wstring code = toLower(text.substr(nccs + 1, ncce - nccs - 1));
		if (code[0] == L'#')
		{
			int32_t cc = parseString< int32_t >(code.substr(1));
			if (cc > 0 && cc <= std::numeric_limits< wchar_t >::max())
				ss << wchar_t(cc);
		}
		else
		{
			if (code == L"nbsp")
				ss << L" ";
			else if (code == L"lt")
				ss << L"<";
			else if (code == L"gt")
				ss << L">";
			else if (code == L"amp")
				ss << L"&";
		}

		offset = ncce + 1;
	}

	ss << text.substr(offset);
	return ss.str();
}

std::wstring encodeCharacterEntities(const std::wstring& text)
{
	// \fixme
	return text;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Text", Text, Node)

Text::Text(const std::wstring& text)
{
	m_text = decodeCharacterEntities(text);
}

std::wstring Text::getName() const
{
	return L"#TEXT#";
}

std::wstring Text::getValue() const
{
	return m_text;
}

void Text::toString(OutputStream& os) const
{
	os << encodeCharacterEntities(m_text);
}

	}
}
