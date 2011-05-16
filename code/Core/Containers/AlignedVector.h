#ifndef traktor_AlignedVector_H
#define traktor_AlignedVector_H

#include <algorithm>
#include <iterator>
#include "Core/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"

namespace traktor
{

/*! \brief Construct/destruct items policy.
 * \ingroup Core
 *
 * Default policy when constructing or destroying
 * items in the AlignedVector container.
 */
template < typename ItemType >
struct AlignedVectorConstructor
{
	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		new (&uninitialized) ItemType(source);
	}

	static void destroy(ItemType& item)
	{
		item.~ItemType();
	}
};

/*! \brief Vector container which strict item alignment.
 * \ingroup Core
 *
 * This container is designed to be a drop-in replacement
 * for STL vector.
 */
template < typename ItemType, typename Constructor = AlignedVectorConstructor< ItemType > >
class AlignedVector
{
	enum
	{
		ExpandSize = 64,
		Alignment = 16
	};

public:
	typedef ItemType value_type;
	typedef int difference_type;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef value_type& reference;

	class const_iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef const value_type* pointer;
		typedef const value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		const_iterator()
		:	m_ptr(0)
		{
		}

		reference operator * () const
		{
			return *m_ptr;
		}

		pointer operator -> () const
		{
			return m_ptr;
		}

		const_iterator operator + (int offset) const
		{
			return const_iterator(m_ptr + offset);
		}

		const_iterator operator - (int offset) const
		{
			return const_iterator(m_ptr - offset);
		}

		void operator += (int offset)
		{
			m_ptr += offset;
		}

		void operator -= (int offset)
		{
			m_ptr -= offset;
		}

		const_iterator operator ++ ()		// pre-fix
		{
			return const_iterator(++m_ptr);
		}

		const_iterator operator ++ (int)	// post-fix
		{
			return const_iterator(m_ptr++);
		}

		const_iterator operator -- ()
		{
			return const_iterator(--m_ptr);
		}

		const_iterator operator -- (int)
		{
			return const_iterator(m_ptr--);
		}

		bool operator == (const const_iterator& r) const
		{
			return m_ptr == r.m_ptr;
		}

		bool operator != (const const_iterator& r) const
		{
			return m_ptr != r.m_ptr;
		}

		bool operator < (const const_iterator& r) const
		{
			return m_ptr < r.m_ptr;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(m_ptr - r.m_ptr);
		}

		const_iterator& operator = (const const_iterator& r)
		{
			m_ptr = r.m_ptr;
			return *this;
		}

		const_pointer _const_ptr() const
		{
			return m_ptr;
		}

	protected:
		friend class AlignedVector;
		ItemType* m_ptr;

		explicit const_iterator(ItemType* ptr)
		:	m_ptr(ptr)
		{
		}
	};

	class iterator : public const_iterator
	{
	public:
		typedef const_iterator _O;
		typedef std::random_access_iterator_tag iterator_category;
		typedef ItemType value_type;
		typedef int difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		reference operator * ()
		{
			return *_O::m_ptr;
		}

		pointer operator -> ()
		{
			return _O::m_ptr;
		}

		iterator operator + (int offset) const
		{
			return iterator(_O::m_ptr + offset);
		}

		iterator operator - (int offset) const
		{
			return iterator(_O::m_ptr - offset);
		}

		iterator operator ++ ()		// pre-fix
		{
			return iterator(++_O::m_ptr);
		}

		iterator operator ++ (int)	// post-fix
		{
			return iterator(_O::m_ptr++);
		}

		iterator operator -- ()
		{
			return iterator(--_O::m_ptr);
		}

		iterator operator -- (int)
		{
			return iterator(_O::m_ptr--);
		}

		bool operator == (const iterator& r) const
		{
			return _O::m_ptr == r.m_ptr;
		}

		bool operator != (const iterator& r) const
		{
			return _O::m_ptr != r.m_ptr;
		}

		bool operator < (const iterator& r) const
		{
			return _O::m_ptr < r.m_ptr;
		}

		difference_type operator - (const const_iterator& r) const
		{
			return difference_type(_O::m_ptr - r._const_ptr());
		}

	protected:
		friend class AlignedVector;

		explicit iterator(ItemType* ptr)
		:	const_iterator(ptr)
		{
		}
	};

	AlignedVector()
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
	}

	AlignedVector(size_t size, const ItemType& value = ItemType())
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		resize(size, value);
	}

	AlignedVector(const AlignedVector< ItemType >& src)
	:	m_data(0)
	,	m_size(0)
	,	m_capacity(0)
	{
		insert(begin(), src.begin(), src.end());
	}

	virtual ~AlignedVector()
	{
		clear();
	}

	/*! \brief Get number of elements in vector.
	 *
	 * \return Number of elements.
	 */
	size_t size() const
	{
		return m_size;
	}

	/*! \brief Get number of elements allocated by vector.
	 *
	 * \return Number of allocated elements.
	 */
	size_t capacity() const
	{
		return m_capacity;
	}

	/*! \brief Check if vector is empty.
	 *
	 * \return True if vector empty.
	 */
	bool empty() const
	{
		return m_size == 0;
	}

	/*! \brief Clear vector.
	 *
	 * Clear frees all memory allocated
	 * by the vector.
	 */
	void clear()
	{
		for (size_t i = 0; i < m_size; ++i)
			Constructor::destroy(m_data[i]);

		getAllocator()->free(m_data);

		m_data = 0;
		m_size = 0;
		m_capacity = 0;
	}

	/*! \brief Resize vector.
	 *
	 * Allocates more elements if required.
	 * If vector shrink then no reallocation is performed.
	 *
	 * \param size New size of vector.
	 */
	void resize(size_t size)
	{
		resize(size, ItemType());
	}

	/*! \brief Resize vector, pad with given value.
	 *
	 * Allocates more elements if required.
	 * If vector shrink then no reallocation is performed.
	 *
	 * \param size New size of vector.
	 * \param pad Pad value.
	 */
	void resize(size_t size, const ItemType& pad)
	{
		if (size > m_size)
		{
			if (size > m_capacity)
			{
				size_t capacity = (size & ~(ExpandSize - 1)) + ExpandSize;
				reserve(capacity);
			}

			for (size_t i = m_size; i < size; ++i)
				Constructor::construct(m_data[i], pad);
		}
		else
		{
			for (size_t i = size; i < m_size; ++i)
				Constructor::destroy(m_data[i]);
		}

		m_size = size;
	}

	/*! \brief Ensure vector capacity.
	 *
	 * \param capacity Vector capacity.
	 */
	void reserve(size_t capacity)
	{
		if (capacity > m_capacity)
		{
			ItemType* data = reinterpret_cast< ItemType* >(getAllocator()->alloc(capacity * sizeof(ItemType), Alignment, T_FILE_LINE));

			if (m_data)
			{
				for (size_t i = 0; i < m_size; ++i)
				{
					Constructor::construct(data[i], m_data[i]);
					Constructor::destroy(m_data[i]);
				}
				getAllocator()->free(m_data);
			}

			m_data = data;
			m_capacity = capacity;
		}
	}

	/*! \brief Push value onto vector.
	 *
	 * \param item Item value.
	 */
	void push_back(const ItemType& item)
	{
		grow(1);
		Constructor::construct(m_data[m_size - 1], item);
	}

	/*! \brief Pop value from vector. */
	void pop_back()
	{
		T_ASSERT (m_size > 0);
		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);
	}

	/*! \brief Swap vector content. */
	void swap(AlignedVector< ItemType >& rh)
	{
		std::swap(m_data, rh.m_data);
		std::swap(m_size, rh.m_size);
		std::swap(m_capacity, rh.m_capacity);
	}

	/*! \brief Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& front()
	{
		T_ASSERT (m_size > 0);
		return *m_data;
	}

	/*! \brief Return reference to value first in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& front() const
	{
		T_ASSERT (m_size > 0);
		return *m_data;
	}

	/*! \brief Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	ItemType& back()
	{
		T_ASSERT (m_size > 0);
		return m_data[m_size - 1];
	}

	/*! \brief Return reference to value last in vector.
	 *
	 * \return Value reference.
	 */
	const ItemType& back() const
	{
		T_ASSERT (m_size > 0);
		return m_data[m_size - 1];
	}

	/*! \brief Return iterator at first element.
	 *
	 * \return Iterator.
	 */
	iterator begin()
	{
		return iterator(m_data);
	}

	/*! \brief Return iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	iterator end()
	{
		return iterator(&m_data[m_size]);
	}

	/*! \brief Return constant iterator at first element.
	 *
	 * \return Iterator.
	 */
	const_iterator begin() const
	{
		return const_iterator(m_data);
	}

	/*! \brief Return constant iterator one step beyond last element.
	 *
	 * \return Iterator.
	 */
	const_iterator end() const
	{
		return const_iterator(&m_data[m_size]);
	}

	/*! \brief Erase element.
	 *
	 * \param where Iterator at element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where)
	{
		size_t offset = size_t(where.m_ptr - m_data);

		for (size_t i = offset; i < m_size - 1; ++i)
			m_data[i] = m_data[i + 1];

		Constructor::destroy(m_data[m_size - 1]);
		shrink(1);

		return iterator(&m_data[offset]);
	}

	/*! \brief Erase range of elements.
	 *
	 * \param where Iterator at element.
	 * \param last Iterator to last element.
	 * \return New iterator at next element.
	 */
	iterator erase(const iterator& where, const iterator& last)
	{
		T_ASSERT (where.m_ptr <= last.m_ptr);

		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(last.m_ptr - where.m_ptr);

		if (count > 0)
		{
			for (size_t i = offset; i < m_size - count; ++i)
				m_data[i] = m_data[i + count];

			for (size_t i = m_size - count; i < m_size; ++i)
				Constructor::destroy(m_data[i]);

			shrink(count);
		}

		return iterator(&m_data[offset]);
	}

	/*! \brief Insert element into vector.
	 *
	 * \param where Iterator at element.
	 * \param item Item value.
	 * \return Iterator at new element.
	 */
	iterator insert(const iterator& where, const ItemType& item)
	{
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);

		grow(1);

		// Initialize grown item.
		Constructor::construct(m_data[size], ItemType());

		// Move items to make room for item to be inserted.
		for (size_t i = size; i > offset; --i)
			m_data[i] = m_data[i - 1];

		// Copy insert item into location.
		Constructor::destroy(m_data[offset]);
		Constructor::construct(m_data[offset], item);

		return iterator(&m_data[offset]);
	}

	/*! \brief Insert elements into vector.
	 *
	 * \param where Iterator at element.
	 * \param from Iterator at first insert element.
	 * \param to Iterator at last insert element.
	 * \return Iterator at new element.
	 */
	iterator insert(const iterator& where, const const_iterator& from, const const_iterator& to)
	{
		size_t size = m_size;
		size_t offset = size_t(where.m_ptr - m_data);
		size_t count = size_t(to.m_ptr - from.m_ptr);

		grow(count);

		// Initialize grown items.
		for (size_t i = 0; i < count; ++i)
			Constructor::construct(m_data[i + size], ItemType());

		// Move items to make room for items to be inserted.
		size_t move = std::min< size_t >(size, count);
		for (size_t i = offset; i < offset + move; ++i)
			m_data[i + count] = m_data[i];

		// Copy insert items into location.
		for (size_t i = 0; i < count; ++i)
		{
			Constructor::destroy(m_data[i + offset]);
			Constructor::construct(m_data[i + offset], from.m_ptr[i]);
		}

		return iterator(&m_data[offset]);
	}

	ItemType& operator [] (size_t index)
	{
		T_ASSERT (index < m_size);
		return m_data[index];
	}

	const ItemType& operator [] (size_t index) const
	{
		T_ASSERT (index < m_size);
		return m_data[index];
	}

	AlignedVector< ItemType >& operator = (const AlignedVector< ItemType >& src)
	{
		clear();
		insert(begin(), src.begin(), src.end());
		return *this;
	}

private:
	ItemType* m_data;
	size_t m_size;
	size_t m_capacity;

	void grow(size_t count)
	{
		size_t newSize = m_size + count;
		if (newSize > m_capacity)
		{
			size_t capacity = (newSize & ~(ExpandSize - 1)) + ExpandSize;
			reserve(capacity);
		}
		m_size = newSize;
	}

	void shrink(size_t count)
	{
		m_size -= count;
	}
};

}

#endif	// traktor_AlignedVector_H
