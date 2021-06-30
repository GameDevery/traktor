#pragma once

#include "Core/Class/Any.h"
#include "Json/JsonNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

/*! JSON named member.
 * \ingroup JSON
 */
class T_DLLCLASS JsonMember : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonMember() = default;

	JsonMember(const std::wstring& name, const Any& value);

	/*! Get name of member.
	 *
	 * \return Name of member.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! Set value of member. */
	void setValue(const Any& value) { m_value = value; }

	/*! Get value of member.
	 *
	 * \return Value of member.
	 */
	const Any& getValue() const { return m_value; }

	virtual bool write(OutputStream& os) const override;

private:
	std::wstring m_name;
	Any m_value;
};

	}
}

