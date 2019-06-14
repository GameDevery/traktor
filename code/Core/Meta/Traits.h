#pragma once

namespace traktor
{

/*! \ingroup Core */
//@{

/*! Is type a pointer. */
template < typename Type >
struct IsPointer
{
	typedef Type type_t;
	typedef Type base_t;

	enum { value = false };
};

/*! Is type a pointer, pointer specialization. */
template < typename Type >
struct IsPointer < Type* >
{
	typedef Type* type_t;
	typedef Type base_t;

	enum { value = true };
};

/*! Is type a pointer, const pointer specialization. */
template < typename Type >
struct IsPointer < const Type* >
{
	typedef const Type* type_t;
	typedef const Type base_t;

	enum { value = true };
};

/*! Is type a pointer, reference specialization. */
template < typename Type >
struct IsPointer < Type& >
{
	typedef Type& type_t;
	typedef Type base_t;

	enum { value = false };
};

/*! Is type a pointer, const reference specialization. */
template < typename Type >
struct IsPointer < const Type& >
{
	typedef const Type& type_t;
	typedef const Type base_t;

	enum { value = false };
};

/*! Is type a reference. */
template < typename Type >
struct IsReference
{
	typedef Type type_t;
	typedef Type base_t;

	enum { value = false };
};

/*! Is type a reference, reference specialization. */
template < typename Type >
struct IsReference < Type& >
{
	typedef Type& type_t;
	typedef Type base_t;

	enum { value = true };
};

/*! Is type a reference, const reference specialization. */
template < typename Type >
struct IsReference < const Type& >
{
	typedef const Type& type_t;
	typedef const Type base_t;

	enum { value = true };
};

/*! Is type const. */
template < typename Type >
struct IsConst
{
	typedef Type type_t;

	enum { value = false };
};

/*! Is type const, specialization. */
template < typename Type >
struct IsConst < const Type >
{
	typedef Type type_t;

	enum { value = true };
};

//@}

}
