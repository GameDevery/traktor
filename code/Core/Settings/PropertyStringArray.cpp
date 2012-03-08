#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyStringArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLONABLE_CLASS(L"traktor.PropertyStringArray", 0, PropertyStringArray, IPropertyValue)

PropertyStringArray::PropertyStringArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringArray::value_type_t PropertyStringArray::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringArray* >(value)->m_value : value_type_t();
}

bool PropertyStringArray::serialize(ISerializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"value", m_value);
}

Ref< IPropertyValue > PropertyStringArray::join(const IPropertyValue* right) const
{
	if (const PropertyStringArray* rightStringArray = dynamic_type_cast< const PropertyStringArray* >(right))
	{
		Ref< PropertyStringArray > leftStringArray = clone_instance(this);
		leftStringArray->m_value.insert(
			leftStringArray->m_value.end(),
			rightStringArray->m_value.begin(),
			rightStringArray->m_value.end()
		);
		return leftStringArray;
	}
	else
		return clone_instance(right);
}

}
