/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Attribute.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Attribute", Attribute, Object)

Attribute::Attribute(const std::wstring& name, const std::wstring& value)
:	m_name(name)
,	m_value(value)
,	m_previous(nullptr)
{
}

const std::wstring& Attribute::getName() const
{
	return m_name;
}

void Attribute::setValue(const std::wstring& value)
{
	m_value = value;
}

const std::wstring& Attribute::getValue() const
{
	return m_value;
}

Attribute* Attribute::getPrevious() const
{
	return m_previous;
}

Attribute* Attribute::getNext() const
{
	return m_next;
}

	}
}
