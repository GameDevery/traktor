#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Member attribute.
 * \ingroup Core
 */
class T_DLLCLASS Attribute : public Object
{
	T_RTTI_CLASS;

public:
	Attribute();

	const Attribute* find(const TypeInfo& type) const;

	const Attribute& operator | (const Attribute& rh);

	template < typename AttributeType >
	const AttributeType* find() const
	{
		return static_cast< const AttributeType* >(
			find(type_of< AttributeType >())
		);
	}

private:
	const Attribute* m_next;
};

}

