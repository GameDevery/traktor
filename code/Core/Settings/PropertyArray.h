/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
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

/*! Property value array.
 * \ingroup Core
 */
class T_DLLCLASS PropertyArray : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef RefArray< IPropertyValue > value_type_t;

	PropertyArray();

	explicit PropertyArray(const RefArray< IPropertyValue >& values);

	static value_type_t get(const IPropertyValue* value);

	void addProperty(IPropertyValue* value);

	void removeProperty(IPropertyValue* value);

	uint32_t getPropertyCount() const;

	IPropertyValue* getProperty(uint32_t index);

	const IPropertyValue* getProperty(uint32_t index) const;

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(uint32_t index, typename PropertyTrait< ValueType >::default_value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(index);
		return value ? PropertyTrait< ValueType >::property_type_t::get(value) : defaultValue;
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(uint32_t index) const
	{
		Ref< const IPropertyValue > value = getProperty(index);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}

	virtual void serialize(ISerializer& s) override final;

	const RefArray< IPropertyValue >& getValues() const { return m_values; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* rightGroup) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

private:
	RefArray< IPropertyValue > m_values;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< RefArray< IPropertyValue > >
{
	typedef PropertyArray property_type_t;
	typedef const RefArray< IPropertyValue >& default_value_type_t;
	typedef RefArray< IPropertyValue > return_type_t;
};

}

