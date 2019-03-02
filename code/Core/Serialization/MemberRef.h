#pragma once

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \brief Object reference member.
 * \ingroup Core
 */
template < typename Class >
class MemberRef : public MemberComplex
{
public:
	typedef Ref< Class > value_type;

	MemberRef(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		Ref< ISerializable > object = (ISerializable*)m_ref.ptr();
		s >> Member< ISerializable* >(
			getName(),
			object,
			AttributeType(type_of< Class >())
		);
		m_ref = checked_type_cast< Class* >(object);
	}

private:
	value_type& m_ref;
};

}

