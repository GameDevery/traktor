/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Script/Editor/Lexer.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

enum CharacterType
{
	CtUnknown = 0,
	CtWhitespace = 1,
	CtAlpha = 2,
	CtDigit = 4,
	CtQuote = 8,
	CtCluster = 16
};

int32_t characterType(wchar_t ch)
{
	if (ch >= L'a' && ch <= L'z')
		return CtAlpha;
	else if (ch >= L'A' && ch <= L'Z')
		return CtAlpha;
	else if (ch == L'_')
		return CtAlpha;
	else if (ch >= L'0' && ch <= L'9')
		return CtDigit;
	else if (ch == L'\"')
		return CtQuote;
	else if (ch == L'.' || ch == L'<' || ch == L'>' || ch == L'=' || ch == L'~')
		return CtCluster;
	else if(ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r')
		return CtWhitespace;
	else
		return CtUnknown;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Lexer", Lexer, Object)

Lexer::Lexer(const wchar_t* text, uint32_t length)
:	m_text(text)
,	m_length(length)
,	m_position(0)
,	m_peek(0)
,	m_number(0.0)
,	m_line(0)
{
}

int32_t Lexer::next()
{
strip_whitespace:

	wchar_t ch = readChar();
	int32_t chtype = characterType(ch);

	// Strip whitespace.
	while ((chtype & CtWhitespace) != 0)
	{
		if (ch == L'\n')
			m_line++;

		if (!(ch = readChar()))
			return LtEndOfFile;

		chtype = characterType(ch);
	}

	// Strip comments.
	if (ch == L'-' && peekChar() == L'-')
	{
		while (ch != L'\n')
		{
			if (!(ch = readChar()))
				return LtEndOfFile;
		}
		m_line++;
		goto strip_whitespace;
	}

	// Parse hexadecimal number.
	if (ch == '0' && peekChar() == L'x')
	{
		StringOutputStream ss;
		ss << L"0x";

		readChar();
		for (;;)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & CtDigit) != 0 || (tolower(ch) >= L'a' && tolower(ch) <= L'f'))
				readChar();
			else
				break;
		}

		m_number = parseString< int32_t >(ss.str());
		return LtNumber;
	}

	// Parse number.
	if (
		(ch == '-' && (characterType(peekChar()) & CtDigit) != 0) ||
		(chtype & CtDigit) != 0
	)
	{
		StringOutputStream ss;

		for (;;)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & CtDigit) != 0)
				readChar();
			else
				break;
		}

		if (peekChar() == L'.')
		{
			readChar();
			ss << L".";

			if (!(ch = peekChar()))
				return LtEndOfFile;
			chtype = characterType(ch);

			while ((chtype & CtDigit) == CtDigit)
			{
				readChar();
				ss << ch;
				ch = peekChar();
				chtype = characterType(ch);
			}
		}

		m_number = parseString< double >(ss.str());
		return LtNumber;
	}

	// Parse word.
	if ((chtype & CtAlpha) != 0)
	{
		StringOutputStream ss;
		while ((chtype & (CtAlpha | CtDigit)) != 0)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & (CtAlpha | CtDigit)) != 0)
				readChar();
		}
		m_word = ss.str();
		return LtWord;
	}

	// Parse quoted string.
	if ((chtype & CtQuote) != 0)
	{
		StringOutputStream ss;
		for (;;)
		{
			wchar_t ch2 = readChar();
			if (ch2 == ch)
				break;
			if (ch2 == L'\\')
			{
				ch2 = readChar();
				switch (ch2)
				{
				case L'n':
					ch2 = L'\n';
					break;
				case L'r':
					ch2 = L'\r';
					break;
				case L't':
					ch2 = L'\t';
					break;
				}
			}
			ss << ch2;
		}
		m_string = ss.str();
		return LtString;
	}

	// Parse cluster.
	if ((chtype & CtCluster) != 0)
	{
		StringOutputStream ss;
		while ((chtype & CtCluster) != 0)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & CtCluster) != 0)
				readChar();
		}
		m_word = ss.str();
		return LtWord;
	}

	return ch;
}

wchar_t Lexer::readChar()
{
	wchar_t ch;
	if (m_peek != 0)
	{
		ch = m_peek;
		m_peek = 0;
	}
	else
	{
		if (m_position < m_length)
			ch = m_text[m_position++];
		else
			ch = 0;
	}
	return ch;
}

wchar_t Lexer::peekChar()
{
	if (m_peek == 0)
		m_peek = readChar();
	return m_peek;
}

	}
}
