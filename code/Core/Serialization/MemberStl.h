#ifndef traktor_MemberStl_H
#define traktor_MemberStl_H

#include <cstring>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief STL vector member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlVector : public MemberArray
{
public:
	typedef std::vector< ValueType > value_type;

	MemberStlVector(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.resize(size);
		m_ref.reserve(capacity);
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}
	
	virtual bool read(ISerializer& s) const
	{
		if (m_index < m_ref.size())
		{
			if (!(s >> ValueMember(L"item", m_ref[m_index])))
				return false;
		}
		else
		{
			ValueType item;
			if (!(s >> ValueMember(L"item", item)))
				return false;
			m_ref.push_back(item);
		}
		++m_index;
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		return s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const
	{
		uint8_t value[sizeof(ValueType)];
		std::memset(value, 0, sizeof(value));

		ValueType* v = new (value) ValueType();
		m_ref.push_back(*v);
		v->~ValueType();

		return true;
	}

private:
	value_type& m_ref;
	mutable size_t m_index;
};

/*! \brief STL list member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlList : public MemberArray
{
public:
	typedef std::list< ValueType > value_type;

	MemberStlList(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}
	
	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.clear();
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(ISerializer& s) const
	{
		m_ref.push_back(ValueType());
		return s >> ValueMember(L"item", m_ref.back());
	}

	virtual bool write(ISerializer& s) const
	{
		return s >> ValueMember(L"item", *m_iter++);
	}

	virtual bool insert() const
	{
		uint8_t value[sizeof(ValueType)];
		std::memset(value, 0, sizeof(value));

		ValueType* v = new (value) ValueType();
		m_ref.push_back(*v);
		v->~ValueType();

		return true;
	}

private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

/*! \brief STL set member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlSet : public MemberArray
{
public:
	typedef std::set< ValueType > value_type;

	MemberStlSet(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}
	
	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.clear();
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(ISerializer& s) const
	{
		ValueType item;
		if (!(s >> ValueMember(L"item", item)))
			return false;

		m_ref.insert(item);
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		return s >> ValueMember(L"item", *m_iter++);
	}

	virtual bool insert() const
	{
		return false;
	}
	
private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

/*! \brief STL pair member. */
template < typename FirstType, typename SecondType, typename FirstMember = Member< FirstType >, typename SecondMember = Member< SecondType > >
class MemberStlPair : public MemberComplex
{
public:
	typedef std::pair< FirstType, SecondType > value_type;

	MemberStlPair(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (!(s >> FirstMember(L"first", m_ref.first)))
			return false;
		if (!(s >> SecondMember(L"second", m_ref.second)))
			return false;
		return true;
	}

private:
	value_type& m_ref;
};

/*! \brief STL map member. */
template < typename KeyType, typename ValueType, typename PairMember = MemberStlPair< KeyType, ValueType > >
class MemberStlMap : public MemberArray
{
public:
	typedef std::map< KeyType, ValueType > value_type;

	MemberStlMap(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.clear();
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(ISerializer& s) const
	{
		typename PairMember::value_type item;
		if (!(s >> PairMember(L"item", item)))
			return false;
		m_ref[item.first] = item.second;
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		typename PairMember::value_type item = std::make_pair(m_iter->first, m_iter->second);
		if (!(s >> PairMember(L"item", item)))
			return false;
		++m_iter;
		return true;
	}

	virtual bool insert() const
	{
		m_ref.insert(std::make_pair< KeyType, ValueType >(
			KeyType(),
			ValueType()
		));
		return true;
	}

private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

//@}

}

#endif	// traktor_MemberStl_H
