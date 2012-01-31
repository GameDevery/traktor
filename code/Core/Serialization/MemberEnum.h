#ifndef traktor_MemberEnum_H
#define traktor_MemberEnum_H

#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Enumeration base member.
 * \ingroup Core
 */
class T_DLLCLASS MemberEnumBase : public MemberComplex
{
public:
	struct Key
	{
		const wchar_t* id;
		int32_t val;
	};

	MemberEnumBase(const wchar_t* const name, const Key* keys);

	const Key* keys() const;

	virtual bool set(const std::wstring& id) const = 0;

	virtual const wchar_t* const get() const = 0;

	virtual bool serialize(ISerializer& s) const;

private:
	const Key* m_keys;
};

/*! \brief Enumeration member.
 * \ingroup Core
 */
template < typename EnumType >
class MemberEnum : public MemberEnumBase
{
public:
	typedef EnumType value_type;

	MemberEnum(const wchar_t* const name, EnumType& en, const Key* keys)
	:	MemberEnumBase(name, keys)
	,	m_en(en)
	{
	}

	virtual bool set(const std::wstring& id) const
	{
		for (const Key* k = keys(); k->id; ++k)
		{
			if (k->id == id)
			{
				m_en = (value_type)k->val;
				return true;
			}
		}
		return false;
	}

	virtual const wchar_t* const get() const
	{
		for (const Key* k = keys(); k->id; ++k)
		{
			if (k->val == m_en)
				return k->id;
		}
		return 0;
	}

private:
	EnumType& m_en;
};

/*! \brief Enumeration member.
 * \ingroup Core
 */
template < typename EnumType, typename ValueType = uint32_t >
class MemberEnumByValue : public MemberComplex
{
public:
	MemberEnumByValue(const wchar_t* const name, EnumType& en)
	:	MemberComplex(name, false)
	,	m_en(en)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		ValueType value = static_cast< ValueType >(m_en);
		if (!(s >> Member< ValueType >(getName(), value)))
			return false;
		m_en = static_cast< EnumType >(value);
		return true;
	}

private:
	EnumType& m_en;
};

}

#endif	// traktor_MemberEnum_H
