#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/IntervalTransform.h"

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
class T_DLLCLASS BoxedIntervalTransform : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedIntervalTransform() = default;

	explicit BoxedIntervalTransform(const IntervalTransform& value);

	Transform get(float interval) const;

	const IntervalTransform& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	IntervalTransform m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < IntervalTransform, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.IntervalTransform";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedIntervalTransform >(value.getObjectUnsafe());
	}
    static Any set(const Transform& value) {
        return Any::fromObject(new BoxedIntervalTransform(value));
    }
    static const IntervalTransform& get(const Any& value) {
		return static_cast< BoxedIntervalTransform* >(value.getObject())->unbox();
    }
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const IntervalTransform&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.IntervalTransform&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedIntervalTransform >(value.getObjectUnsafe());
	}
    static Any set(const IntervalTransform& value) {
        return Any::fromObject(new BoxedIntervalTransform(value));
    }
    static const IntervalTransform& get(const Any& value) {
		return static_cast< BoxedIntervalTransform* >(value.getObject())->unbox();
    }
};

}
