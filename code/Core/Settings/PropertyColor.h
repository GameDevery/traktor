/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PropertyColor_H
#define traktor_PropertyColor_H

#include "Core/Math/Color4ub.h"
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

/*! \brief Color property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyColor : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Color4ub value_type_t;

	PropertyColor(const value_type_t& value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) override final;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

private:
	value_type_t m_value;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< Color4ub >
{
	typedef PropertyColor property_type_t;
	typedef const Color4ub& default_value_type_t;
	typedef Color4ub return_type_t;
};

}

#endif	// traktor_PropertyColor_H
