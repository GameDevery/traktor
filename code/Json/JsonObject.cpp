/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Json/JsonArray.h"
#include "Json/JsonMember.h"
#include "Json/JsonObject.h"

namespace traktor::json
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonObject", JsonObject, JsonNode)

void JsonObject::push(JsonMember* member)
{
	m_members.push_back(member);
}

JsonMember* JsonObject::getMember(const std::wstring& name) const
{
	for (auto member : m_members)
	{
		if (member->getName() == name)
			return member;
	}
	return nullptr;
}

void JsonObject::setMemberValue(const std::wstring& name, const Any& value)
{
	JsonMember* member = getMember(name);
	if (member)
		member->setValue(value);
	else
		push(new JsonMember(name, value));
}

Any JsonObject::getMemberValue(const std::wstring& name) const
{
	const JsonMember* member = getMember(name);
	return member ? member->getValue() : Any();
}

Any JsonObject::getValue(const std::wstring& path) const
{
	Any iter = Any::fromObject(const_cast< JsonObject* >(this));

	StringSplit< std::wstring > s(path, L".");
	for (auto i = s.begin(); i != s.end(); ++i)
	{
		if (auto nodeObject = dynamic_type_cast< JsonObject* >(iter.getObject()))
		{
			JsonMember* member = nodeObject->getMember(*i);
			if (member)
				iter = member->getValue();
			else
				return Any();
		}
		else if (auto nodeArray = dynamic_type_cast< JsonArray* >(iter.getObject()))
		{
			uint32_t index = parseString< int32_t >(*i, ~0U);
			if (index < nodeArray->size())
				iter = nodeArray->get(index);
			else
				return Any();
		}
	}

	return iter;
}

bool JsonObject::write(OutputStream& os) const
{
	os << L"{" << Endl;
	os << IncreaseIndent;

	for (auto it = m_members.begin(); it != m_members.end(); ++it)
	{
		if (it != m_members.begin())
			os << L"," << Endl;

		if (!(*it)->write(os))
			return false;
	}
	if (!m_members.empty())
		os << Endl;

	os << DecreaseIndent;
	os << L"}" << Endl;
	return true;
}

}
