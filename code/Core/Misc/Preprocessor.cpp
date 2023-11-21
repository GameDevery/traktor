/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stack>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Preprocessor.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"

namespace traktor
{
	namespace
	{

enum PreprocessorKeyword
{
	PkwUnknown,
	PkwIf,
	PkwElseIf,
	PkwElse,
	PkwEnd,
	PkwUsing,
	PkwDefine
};

struct Tokenizer
{
	enum Token
	{
		TkUnknown = 0,
		TkEnd = -1,
		TkWord = -2,
		TkNumber = -3
	};

	std::wstring expression;
	std::wstring word;
	int32_t numchr;
	int32_t current;
	int32_t token;
	int32_t pushed;

	Tokenizer(const std::wstring& expression_)
	:	expression(expression_)
	,	numchr(0)
	,	current(0)
	,	token(0)
	,	pushed(0)
	{
	}

	int32_t next()
	{
		if (pushed > 0)
		{
			--pushed;
			return token;
		}

		if (current >= expression.size())
			return (token = TkEnd);

		for (;;)
		{
			numchr = expression[current++];
			if (numchr == L' ' || numchr == L'\t')
			{
				if (current >= expression.size())
					return (token = TkEnd);
			}
			else
				break;
		}

		if (std::wstring(L"()+-*/!=").find(numchr) != std::wstring::npos)
			return numchr;

		if (numchr >= L'0' && numchr <= L'9')
		{
			int32_t n = numchr - L'0';
			while (current < expression.size())
			{
				if (expression[current] >= L'0' && expression[current] <= L'9')
				{
					n *= 10;
					n += expression[current++] - L'0';
				}
				else
					break;
			}

			numchr = n;
			return (token = TkNumber);
		}

		else if (
			numchr == L'_' ||
			(numchr >= L'a' && numchr <= L'z') ||
			(numchr >= L'A' && numchr <= L'Z')
		)
		{
			word = L"";
			word.push_back(numchr);

			while (current < expression.size())
			{
				if (
					expression[current] == L'_' ||
					(expression[current] >= L'a' && expression[current] <= L'z') ||
					(expression[current] >= L'A' && expression[current] <= L'Z') ||
					(expression[current] >= L'0' && expression[current] <= L'9')
				)
					word.push_back(expression[current++]);
				else
					break;
			}

			return (token = TkWord);
		}

		return (token = TkUnknown);
	}

	void push()
	{
		++pushed;
	}
};

int32_t evaluateL(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error);

int32_t evaluateAtoms(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	int32_t value = 0;
	switch (t.next())
	{
	case L'(':
		{
			value = evaluateL(t, definitions, error);
			error |= (t.next() != L')');
		}
		break;

	case L'+':
		{
			value = evaluateAtoms(t, definitions, error);
		}
		break;

	case L'-':
		{
			value = -evaluateAtoms(t, definitions, error);
		}
		break;

	case L'!':
		{
			value = (evaluateAtoms(t, definitions, error) != 0) ? 0 : 1;
		}
		break;

	case Tokenizer::TkWord:
		{
			std::wstring fn = t.word;

			if (t.next() == L'(')
			{
				error |= (t.next() != Tokenizer::TkWord);

				std::wstring arg = t.word;

				error |= (t.next() != L')');

				if (!error)
				{
					if (fn == L"defined")
					{
						const auto i = definitions.find(arg);
						value = (i != definitions.end()) ? 1 : 0;
					}
					else
						error = true;
				}
			}
			else
			{
				t.push();
				const auto i = definitions.find(fn);
				if (i != definitions.end())
					value = i->second.getInt32();
				else
					value = 0;
			}
		}
		break;

	case Tokenizer::TkNumber:
		{
			value = t.numchr;
		}
		break;
	}
	return value;
}

int32_t evaluateFactors(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	int32_t left = evaluateAtoms(t, definitions, error);
	while (!error)
	{
		int32_t tok = t.next();
		if (tok == L'*')
			left *= evaluateAtoms(t, definitions, error);
		else if (tok == L'/')
			left /= evaluateAtoms(t, definitions, error);
		else
		{
			t.push();
			break;
		}
	}
	return left;
}

int32_t evaluateSummands(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	int32_t left = evaluateFactors(t, definitions, error);
	while (!error)
	{
		int32_t tok = t.next();
		if (tok == L'+')
			left += evaluateFactors(t, definitions, error);
		else if (tok == L'-')
			left |= evaluateFactors(t, definitions, error);
		else
		{
			t.push();
			break;
		}
	}
	return left;
}

int32_t evaluateBitwise(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	int32_t left = evaluateSummands(t, definitions, error);
	while (!error)
	{
		int32_t tok = t.next();
		if (tok == L'&')
			left &= evaluateSummands(t, definitions, error);
		else if (tok == L'|')
			left |= evaluateSummands(t, definitions, error);
		else
		{
			t.push();
			break;
		}
	}
	return left;
}

int32_t evaluateLogical(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	int32_t left = evaluateBitwise(t, definitions, error);
	while (!error)
	{
		int32_t tok = t.next();
		if (tok == L'&')
		{
			error |= (t.next() != L'&');
			left = ((left > 0) && (evaluateBitwise(t, definitions, error) > 0)) ? 1 : 0;
		}
		else if (tok == L'|')
		{
			error |= (t.next() != L'|');
			left = ((left > 0) || (evaluateBitwise(t, definitions, error) > 0)) ? 1 : 0;
		}
		else if (tok == L'=')
		{
			error |= (t.next() != L'=');
			left = (left == evaluateBitwise(t, definitions, error)) ? 1 : 0;
		}
		else if (tok == L'!')
		{
			error |= (t.next() != L'=');
			left = (left != evaluateBitwise(t, definitions, error)) ? 1 : 0;
		}
		else
		{
			t.push();
			break;
		}
	}
	return left;
}

int32_t evaluateL(Tokenizer& t, const std::map< std::wstring, Any >& definitions, bool& error)
{
	return evaluateLogical(t, definitions, error);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Preprocessor", Preprocessor, Object)

void Preprocessor::setDefinition(const std::wstring& symbol, const Any& value)
{
	m_definitions[symbol] = value;
}

void Preprocessor::removeDefinition(const std::wstring& symbol)
{
	auto it = m_definitions.find(symbol);
	if (it != m_definitions.end())
		m_definitions.erase(it);
}

bool Preprocessor::evaluate(const std::wstring& source, std::wstring& output, std::set< std::wstring >& usings) const
{
	StringOutputStream ss;

	// Create a local duplicate of definitions so evaluation can modify it.
	std::map< std::wstring, Any > definitions = m_definitions;

	std::stack< int32_t > keep;
	keep.push(0);

	StringSplit< std::wstring > split(source, L"\n");
	for (StringSplit< std::wstring >::const_iterator i = split.begin(); i != split.end(); ++i)
	{
		std::wstring line = trim(*i);
		if (!line.empty() && line[0] == L'#')
		{
			int32_t keyword = PkwUnknown;
			std::wstring expression;

			ss << Endl;

			const size_t sep = line.find_first_of(L" \t");
			if (sep != line.npos)
			{
				const std::wstring tmp = line.substr(0, sep);

				if (tmp == L"#if")
					keyword = PkwIf;
				else if (tmp == L"#elif")
					keyword = PkwElseIf;
				else if (tmp == L"#using")
					keyword = PkwUsing;
				else if (tmp == L"#define")
					keyword = PkwDefine;

				expression = line.substr(sep + 1);
			}
			else
			{
				if (line == L"#else")
					keyword = PkwElse;
				else if (line == L"#endif")
					keyword = PkwEnd;
			}

			switch (keyword)
			{
			case PkwIf:
				{
					if (keep.top() == 0)
						keep.push(
							(evaluateExpression(expression, definitions) != 0) ? 0 : 1
						);
					else
						keep.push(2);
				}
				break;

			case PkwElseIf:
				{
					if (keep.top() == 0)
						keep.top() = 2;
					else if (keep.top() == 1)
						keep.top() = (evaluateExpression(expression, definitions) != 0) ? 0 : 1;
				}
				break;

			case PkwElse:
				{
					if (keep.top() == 0)
						keep.top() = 2;
					else if (keep.top() == 1)
						keep.top() = 0;
				}
				break;

			case PkwEnd:
				{
					keep.pop();
					if (keep.empty())
					{
						log::error << L"Preprocessor failed; unbalanced #if/endif" << Endl;
						return false;
					}
				}
				break;

			case PkwUsing:
				if (keep.top() == 0)
					usings.insert(trim(expression));
				break;

			case PkwDefine:
				if (keep.top() == 0)
				{
					const size_t sep = expression.find_first_of(L" \t");
					const std::wstring def = expression.substr(0, sep);
					if (sep != expression.npos)
					{
						const std::wstring xpr = trim(expression.substr(sep + 1));
						definitions[trim(def)] = Any::fromString(xpr);
					}
					else
						definitions[trim(def)] = Any();
				}
				break;

			default:
				{
					log::error << L"Preprocessor failed; unknown keyword" << Endl;
					return false;
				}
				break;
			}
		}
		else
		{
			if (keep.top() == 0)
				ss << *i << Endl;
			else
				ss << Endl;
		}
	}

	keep.pop();
	if (!keep.empty())
	{
		log::error << L"Preprocessor failed; unbalanced #if/endif" << Endl;
		return false;
	}

	output = ss.str();
	return true;
}

int32_t Preprocessor::evaluateExpression(const std::wstring& expression, const std::map< std::wstring, Any >& definitions) const
{
	Tokenizer t(expression);
	bool error = false;

	int32_t result = evaluateL(t, definitions, error);
	if (error)
	{
		log::error << L"Preprocessor failed; syntax error in expression \"" << expression << L"\"" << Endl;
		return 0;
	}

	return result;
}

}