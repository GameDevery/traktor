#ifndef traktor_PropertyString_H
#define traktor_PropertyString_H

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

/*! \brief String property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyString : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::wstring value_type_t;

	PropertyString(value_type_t value = L"");

	static value_type_t get(const IPropertyValue* value);

	virtual bool serialize(ISerializer& s);

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const;

private:
	value_type_t m_value;
};

}

#endif	// traktor_PropertyString_H
