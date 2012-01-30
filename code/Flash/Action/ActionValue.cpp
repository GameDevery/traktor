#include <algorithm>
#include <cstring>
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Classes/Boolean.h"
#include "Flash/Action/Classes/Number.h"
#include "Flash/Action/Classes/String.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if defined(_DEBUG)
#	define T_VALIDATE(av) \
	T_ASSERT ((av).getType() >= ActionValue::AvtUndefined && (av).getType() <= ActionValue::AvtObject);
#else
#	define T_VALIDATE(av)
#endif

static int32_t s_stringCount = 0;

char* refStringCreate(const char* s)
{
	uint32_t len = strlen(s);
	T_ASSERT (len < 4096);
	
	void* ptr = getAllocator()->alloc(sizeof(uint16_t) + (len + 1) * sizeof(char), 4, T_FILE_LINE);
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 1;

	char* c = reinterpret_cast< char* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(char));

	c[len] = '\0';

	++s_stringCount;
	return c;
}

char* refStringInc(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	(*base)++;
	return s;
}

char* refStringDec(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	if (--(*base) == 0)
	{
		getAllocator()->free(base);
		--s_stringCount;
		return 0;
	}
	return s;
}

		}

ActionValue::ActionValue()
:	m_type(AvtUndefined)
{
	m_value.o = 0;
}

ActionValue::ActionValue(const ActionValue& v)
:	m_type(v.m_type)
{
	T_VALIDATE(v);
	m_value.o = 0;
	if (m_type == AvtString)
		m_value.s = refStringInc(v.m_value.s);
	else if (m_type == AvtObject)
	{
		T_SAFE_ADDREF (v.m_value.o);
		m_value.o = v.m_value.o;
	}
	else
		m_value = v.m_value;
}

ActionValue::ActionValue(bool b)
:	m_type(AvtBoolean)
{
	m_value.o = 0;
	m_value.b = b;
}

ActionValue::ActionValue(avm_number_t n)
:	m_type(AvtNumber)
{
	m_value.o = 0;
	m_value.n = n;
}

ActionValue::ActionValue(const char* s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s);
}

ActionValue::ActionValue(const std::string& s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s.c_str());
}

ActionValue::ActionValue(const wchar_t* s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str());
}

ActionValue::ActionValue(const std::wstring& s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str());
}

ActionValue::ActionValue(ActionObject* o)
:	m_type(AvtObject)
{
	T_SAFE_ADDREF(o);
	m_value.o = o;
}

ActionValue::~ActionValue()
{
	T_EXCEPTION_GUARD_BEGIN

	T_VALIDATE(*this);
	if (m_type == AvtString && m_value.s)
		refStringDec(m_value.s);
	else if (m_type == AvtObject && m_value.o)
		T_SAFE_RELEASE (m_value.o);

	T_EXCEPTION_GUARD_END
}

bool ActionValue::getBoolean() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b;

	case AvtNumber:
		return bool(m_value.n != 0.0);

	case AvtString:
		return std::strlen(m_value.s) > 0;

	case AvtObject:
		return bool(m_value.o != 0);

	default:
		break;
	}
	return false;
}

avm_number_t ActionValue::getNumber() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? avm_number_t(1) : avm_number_t(0);

	case AvtNumber:
		return m_value.n;

	case AvtObject:
		return m_value.o ? m_value.o->valueOf().getNumber() : avm_number_t(0);

	default:
		break;
	}
	return avm_number_t(0);
}

std::string ActionValue::getString() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? "true" : "false";

	case AvtNumber:
		return wstombs(traktor::toString(m_value.n));

	case AvtString:
		return m_value.s;

	case AvtObject:
		return m_value.o ? m_value.o->toString().getString() : "null";

	default:
		break;
	}
	return "undefined";
}

std::wstring ActionValue::getWideString() const
{
	return mbstows(Utf8Encoding(), getString());
}

Ref< ActionObject > ActionValue::getObjectAlways(ActionContext* context) const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return (new Boolean(m_value.b))->getAsObject(context);

	case AvtNumber:
		return (new Number(m_value.n))->getAsObject(context);

	case AvtString:
		return (new String(m_value.s))->getAsObject(context);

	case AvtObject:
		return m_value.o ? m_value.o : new ActionObject(context);

	default:
		break;
	}
	return new ActionObject(context);
}

bool ActionValue::serialize(ISerializer& s)
{
	const MemberEnum< Type >::Key kType[] =
	{
		{ L"AvtUndefined", AvtUndefined },
		{ L"AvtBoolean", AvtBoolean },
		{ L"AvtNumber", AvtNumber },
		{ L"AvtString", AvtString },
		{ L"AvtObject", AvtObject },
		{ 0, 0 }
	};

	s >> MemberEnum< Type >(L"type", m_type, kType);
	switch (m_type)
	{
	case AvtUndefined:
		break;

	case AvtBoolean:
		s >> Member< bool >(L"value", m_value.b);
		break;

	case AvtNumber:
		s >> Member< avm_number_t >(L"value", m_value.n);
		break;

	case AvtString:
		{
			if (s.getDirection() == ISerializer::SdRead)
			{
				std::string str;
				s >> Member< std::string >(L"value", str);
				m_value.s = refStringCreate(str.c_str());
			}
			else
			{
				std::string str = m_value.s;
				s >> Member< std::string >(L"value", str);
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

ActionValue& ActionValue::operator = (const ActionValue& v)
{
	T_VALIDATE(*this);
	T_VALIDATE(v);

	if (v.m_type == AvtString)
		refStringInc(v.m_value.s);
	else if (v.m_type == AvtObject)
		T_SAFE_ADDREF(v.m_value.o);

	if (m_type == AvtString)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		T_SAFE_RELEASE(m_value.o);

	m_type = v.m_type;
	m_value = v.m_value;

	return *this;
}

ActionValue ActionValue::operator + (const ActionValue& r) const
{
	if (r.isString() || isString())
	{
		ActionValue string2 = r.toString();
		ActionValue string1 = toString();
		if (string2.isString() && string1.isString())
		{
			std::string str2 = string2.getString();
			std::string str1 = string1.getString();
			return ActionValue(str1 + str2);
		}
		else
			return ActionValue();
	}
	else
	{
		ActionValue number2 = r.toNumber();
		ActionValue number1 = toNumber();
		if (number2.isNumeric() && number1.isNumeric())
		{
			avm_number_t n2 = number2.getNumber();
			avm_number_t n1 = number1.getNumber();
			return ActionValue(n1 + n2);
		}
		else
			return ActionValue();
	}
}

ActionValue ActionValue::operator - (const ActionValue& r) const
{
	if (isNumeric() && r.isNumeric())
		return ActionValue(getNumber() - r.getNumber());
	else
		return ActionValue();
}

ActionValue ActionValue::operator * (const ActionValue& r) const
{
	if (isNumeric() && r.isNumeric())
		return ActionValue(getNumber() * r.getNumber());
	else
		return ActionValue();
}

bool ActionValue::operator == (const ActionValue& r) const
{
	if (!r.isUndefined() && !isUndefined())
	{
		ActionValue::Type predicateType = std::max(r.getType(), getType());
		if (predicateType == ActionValue::AvtBoolean)
		{
			bool v2 = r.getBoolean();
			bool v1 = getBoolean();
			return v1 == v2;
		}
		else if (predicateType == ActionValue::AvtNumber)
		{
			avm_number_t v2 = r.getNumber();
			avm_number_t v1 = getNumber();
			return v1 == v2;
		}
		else if (predicateType == ActionValue::AvtString)
		{
			std::string v2 = r.getString();
			std::string v1 = getString();
			return v1 == v2;
		}
		else	// AvtObject
		{
			if (r.isObject() && isObject())
			{
				Ref< ActionObject > object2 = r.getObject();
				Ref< ActionObject > object1 = getObject();
				return object1 == object2;
			}
			else
				return false;
		}
	}
	else if (isObject() && r.isUndefined())
	{
		ActionObject* object1 = getObject();
		return object1 == 0;
	}
	else if (isUndefined() && r.isObject())
	{
		ActionObject* object2 = r.getObject();
		return object2 == 0;
	}
	else if (isUndefined() && r.isUndefined())
		return true;
	else
		return false;
}

	}
}
