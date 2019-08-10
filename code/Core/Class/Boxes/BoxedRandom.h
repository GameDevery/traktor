#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Random.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedRandom : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRandom();

	explicit BoxedRandom(const Random& value);

	explicit BoxedRandom(uint32_t seed);

	uint32_t next() { return m_value.next(); }

	float nextFloat() { return m_value.nextFloat(); }

	const Random& unbox() const { return m_value; }

	virtual std::wstring toString() const override;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Random m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Random, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Random";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static const Random& get(const Any& value) {
		return static_cast< BoxedRandom* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Random&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Random&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static const Random& get(const Any& value) {
		return static_cast< BoxedRandom* >(value.getObject())->unbox();
	}
};

}
