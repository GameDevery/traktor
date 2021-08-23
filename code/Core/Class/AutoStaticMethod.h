#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

template < typename ClassType, typename ReturnType, typename ... ArgumentTypes >
class AutoStaticMethod final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoStaticMethod);

public:
	typedef typename MethodSignature< false, ClassType, ReturnType, ArgumentTypes ... >::static_method_t static_method_t;

	static_method_t m_method;

	explicit AutoStaticMethod(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final 
	{
		StringOutputStream ss;
		ss << CastAny< ReturnType >::typeName();
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { ( ss << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
		return ss.str();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		return invokeI(argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
	}

private:
	template < std::size_t... Is >
	inline Any invokeI(const Any* argv, std::index_sequence< Is... >) const
	{
		return CastAny< ReturnType >::set((*m_method)(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		));
	}
};

template < typename ClassType, typename ... ArgumentTypes >
class AutoStaticMethod < ClassType, void, ArgumentTypes ... > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoStaticMethod);

public:
	typedef typename MethodSignature< false, ClassType, void, ArgumentTypes ... >::static_method_t static_method_t;

	static_method_t m_method;

	explicit AutoStaticMethod(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final 
	{
		StringOutputStream ss;
		ss << L"void";
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { ( ss << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
		return ss.str();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		invokeI(argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
		return Any();
	}

private:
	template < std::size_t... Is >
	inline void invokeI(const Any* argv, std::index_sequence< Is... >) const
	{
		(*m_method)(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		);
	}
};

/*! \} */

}
