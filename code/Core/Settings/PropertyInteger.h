#ifndef traktor_PropertyInteger_H
#define traktor_PropertyInteger_H

#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Integer property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyInteger : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef int32_t value_type_t;

	PropertyInteger(value_type_t value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

}

#endif	// traktor_PropertyInteger_H
