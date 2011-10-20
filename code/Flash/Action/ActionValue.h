#ifndef traktor_flash_ActionValue_H
#define traktor_flash_ActionValue_H

#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/TString.h"
#include "Flash/Action/ActionTypes.h"
#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionObject;

/*! \brief Action value.
 * \ingroup Flash
 *
 * An action value can be of several different
 * types and are dynamically cast as needed.
 */
class T_DLLCLASS ActionValue
{
public:
	enum Type
	{
		AvtUndefined,
		AvtBoolean,
		AvtNumber,
		AvtString,
		AvtObject
	};

	ActionValue();

	ActionValue(const ActionValue& v);

	explicit ActionValue(bool b);

	explicit ActionValue(avm_number_t n);

	explicit ActionValue(const char* s);

	explicit ActionValue(const std::string& s);

	explicit ActionValue(const wchar_t* s);

	explicit ActionValue(const std::wstring& s);

	explicit ActionValue(ActionObject* o);

	virtual ~ActionValue();

	/*! \brief Cast to boolean. */
	ActionValue toBoolean() const { return ActionValue(getBoolean()); }

	/*! \brief Cast to number. */
	ActionValue toNumber() const { return ActionValue(getNumber()); }

	/*! \brief Cast to string. */
	ActionValue toString() const { return ActionValue(getString()); }

	/*! \brief Get type of value. */
	Type getType() const { return m_type; }

	/*! \brief Check if undefined. */
	bool isUndefined() const { return m_type == AvtUndefined; }

	/*! \brief Check if boolean. */
	bool isBoolean() const { return m_type == AvtBoolean; }

	/*! \brief Check if number. */
	bool isNumeric() const { return m_type == AvtNumber; }

	/*! \brief Check if string. */
	bool isString() const { return m_type == AvtString; }

	/*! \brief Check if object. */
	bool isObject() const { return m_type == AvtObject; }

	/*! \brief Check if object. */
	template < typename ObjectType >
	bool isObject() const { return (m_type == AvtObject) && is_a< ObjectType >(m_value.o); }

	/*! \brief Get boolean value. */
	bool getBoolean() const;

	/*! \brief Get number value. */
	avm_number_t getNumber() const;

	/*! \brief Get string value. */
	std::string getString() const;

	/*! \brief Get wide string value. */
	std::wstring getWideString() const;

	/*! \brief Get object. */
	ActionObject* getObject() const { return (m_type == AvtObject) ? m_value.o : 0; }

	/*! \brief Get object. */
	template < typename ObjectType >
	ObjectType* getObject() const { return dynamic_type_cast< ObjectType* >(getObject()); }

	/*! \brief Get object always, ie. create boxes if not a object. */
	Ref< ActionObject > getObjectAlways(ActionContext* context) const;

	/*! \brief Get object always, ie. create boxes if not a object. */
	template < typename ObjectType >
	Ref< ObjectType > getObjectAlways(ActionContext* context) const { return dynamic_type_cast< ObjectType* >(getObjectAlways(context)); }

	/*! \brief Copy value. */
	ActionValue& operator = (const ActionValue& v);

private:
	union Value
	{
		bool b;
		avm_number_t n;
		char* s;
		ActionObject* o;
	};

	Type m_type;
	Value m_value;
};

	}
}

#endif	// traktor_flash_ActionValue_H
