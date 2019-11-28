#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

/*! \name Properties */
/*! \{ */

template <
	typename ClassType,
	typename ValueType
>
struct PropertySet final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(PropertySet);

	typedef typename MethodSignature< false, ClassType, void, ValueType >::method_t method_t;

	method_t m_method;

	PropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const override final
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		((ClassType*)(self)->*m_method)(
			CastAny< ValueType >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ValueType,
	bool Const
>
struct PropertyGet final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(PropertyGet);

	typedef typename MethodSignature< Const, ClassType, ValueType >::method_t method_t;

	method_t m_method;

	PropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const override final
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		ValueType value = ((ClassType*)(self)->*m_method)();
		return CastAny< ValueType >::set(value);
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertySet final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnPropertySet);

	typedef void (*method_t)(ClassType*, ValueType);

	method_t m_method;

	FnPropertySet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const override final
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		(*m_method)(
			(ClassType*)(self),
			CastAny< ValueType >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ValueType
>
struct FnPropertyGet final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnPropertyGet);

	typedef ValueType (*method_t)(ClassType*);

	method_t m_method;

	FnPropertyGet(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const override final
	{
		CastAny< ValueType >::typeName(ss);
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		ValueType value = (*m_method)((ClassType*)(self));
		return CastAny< ValueType >::set(value);
	}
};

/*! \} */

/*! \} */

}
