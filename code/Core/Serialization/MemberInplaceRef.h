#ifndef traktor_MemberInplaceRef_H
#define traktor_MemberInplaceRef_H

#include "Core/InplaceRef.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \brief Object reference member.
 * \ingroup Core
 */
template < typename Class >	
class MemberInplaceRef : public MemberComplex
{
public:
	typedef InplaceRef< Class > value_type;
	
	MemberInplaceRef(const wchar_t* const name, value_type ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		Ref< ISerializable > object = (ISerializable*)m_ref.ptr();
		if (!(s >> Member< ISerializable* >(
			getName(),
			object,
			AttributeType(type_of< Class >())
		)))
			return false;
		m_ref = checked_type_cast< Class* >(object.ptr());
		return true;
	}
	
private:
	value_type m_ref;
};

}

#endif	// traktor_MemberInplaceRef_H
