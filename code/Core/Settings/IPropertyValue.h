#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Property value base.
 * \ingroup Core
 */
class T_DLLCLASS IPropertyValue : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const = 0;

	virtual Ref< IPropertyValue > clone() const = 0;
};

/*!
 * \ingroup Core
 */
template< typename ValueType >
struct PropertyTrait {};

}

