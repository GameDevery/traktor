#ifndef traktor_script_Boxes_H
#define traktor_script_Boxes_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Script/Any.h"
#include "Script/CastAny.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS BoxedUInt64 : public Object
{
	T_RTTI_CLASS;

public:
	BoxedUInt64();

	explicit BoxedUInt64(uint64_t value);

	uint64_t unbox() const { return m_value; }

private:
	uint64_t m_value;
};

class T_DLLCLASS BoxedGuid : public Object
{
	T_RTTI_CLASS;

public:
	BoxedGuid();

	explicit BoxedGuid(const Guid& value);

	explicit BoxedGuid(const std::wstring& value);

	void set(const std::wstring& str) { m_value = Guid(str); }

	std::wstring format() const { return m_value.format(); }

	bool isValid() const { return m_value.isValid(); }

	bool isNull() const { return m_value.isNull(); }

	bool isNotNull() const { return m_value.isNotNull(); }

	const Guid& unbox() const { return m_value; }

private:
	Guid m_value;
};

class T_DLLCLASS BoxedVector2 : public Object
{
	T_RTTI_CLASS;

public:
	BoxedVector2();

	explicit BoxedVector2(const Vector2& value);

	explicit BoxedVector2(float x, float y);

	void set(float x, float y) { m_value.set(x, y); }

	float x() const { return m_value.x; }

	float y() const { return m_value.y; }

	Vector2 add(const Vector2& v) const;

	Vector2 sub(const Vector2& v) const;

	Vector2 mul(const Vector2& v) const;

	Vector2 div(const Vector2& v) const;

	Vector2 add(float v) const;

	Vector2 sub(float v) const;

	Vector2 mul(float v) const;

	Vector2 div(float v) const;

	const Vector2& unbox() const { return m_value; }

private:
	Vector2 m_value;
};

class T_DLLCLASS BoxedVector4 : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedVector4();
	
	explicit BoxedVector4(const Vector4& value);

	explicit BoxedVector4(float x, float y, float z);
	
	explicit BoxedVector4(float x, float y, float z, float w);

	void set(float x, float y, float z, float w);
	
	float x() const { return m_value.x(); }
	
	float y() const { return m_value.y(); }
	
	float z() const { return m_value.z(); }
	
	float w() const { return m_value.w(); }

	Vector4 xyz0() const { return m_value.xyz0(); }

	Vector4 xyz1() const { return m_value.xyz1(); }

	Vector4 add(const Vector4& v) const;

	Vector4 sub(const Vector4& v) const;

	Vector4 mul(const Vector4& v) const;

	Vector4 div(const Vector4& v) const;

	Vector4 add(float v) const;

	Vector4 sub(float v) const;

	Vector4 mul(float v) const;

	Vector4 div(float v) const;

	float dot(const Vector4& v) const;

	Vector4 cross(const Vector4& v) const;

	float length() const;

	Vector4 normalized() const;

	Vector4 neg() const;
	
	const Vector4& unbox() const { return m_value; }
	
private:
	Vector4 m_value;
};

class T_DLLCLASS BoxedQuaternion : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const Vector4& axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const Vector4& from, const Vector4& to);
	
	float x() const { return m_value.e.x(); }
	
	float y() const { return m_value.e.y(); }
	
	float z() const { return m_value.e.z(); }
	
	float w() const { return m_value.e.w(); }

	Quaternion normalized() const;

	Quaternion inverse() const;

	Quaternion concat(const Quaternion& q) const;

	Vector4 transform(const Vector4& v) const;

	Vector4 getEulerAngles() const;

	Vector4 getAxisAngle() const;
	
	const Quaternion& unbox() const { return m_value; }

private:
	Quaternion m_value;
};

class T_DLLCLASS BoxedTransform : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedTransform();
	
	explicit BoxedTransform(const Transform& value);
	
	explicit BoxedTransform(const Vector4& translation, const Quaternion& rotation);
	
	const Vector4& translation() const;
	
	const Quaternion& rotation() const;

	Vector4 axisX() const;

	Vector4 axisY() const;

	Vector4 axisZ() const;

	Transform inverse() const;

	Transform concat(const Transform& t) const;

	Vector4 transform(const Vector4& v) const;
	
	const Transform& unbox() const { return m_value; }

private:
	Transform m_value;
};

class T_DLLCLASS BoxedAabb3 : public Object
{
	T_RTTI_CLASS;

public:
	BoxedAabb3();

	explicit BoxedAabb3(const Aabb3& value);

	explicit BoxedAabb3(const Vector4& min, const Vector4& max);

	bool inside(const Vector4& pt) const { return m_value.inside(pt); }

	Aabb3 transform(const Transform& tf) const { return m_value.transform(tf); }

	Vector4 getCenter() const { return m_value.getCenter(); }

	Vector4 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	bool overlap(const Aabb3& aabb) const { return m_value.overlap(aabb); }

	const Aabb3& unbox() const { return m_value; }

private:
	Aabb3 m_value;
};

class T_DLLCLASS BoxedColor4f : public Object
{
	T_RTTI_CLASS;

public:
	BoxedColor4f();

	explicit BoxedColor4f(const Color4f& value);

	explicit BoxedColor4f(float red, float green, float blue);

	explicit BoxedColor4f(float red, float green, float blue, float alpha);

	float get(int32_t channel) const { return m_value.get(channel); }

	float getRed() const { return m_value.getRed(); }

	float getGreen() const { return m_value.getGreen(); }

	float getBlue() const { return m_value.getBlue(); }

	float getAlpha() const { return m_value.getAlpha(); }

	void set(int32_t channel, float value) { m_value.set(channel, Scalar(value)); }

	void setRed(float red) { m_value.setRed(Scalar(red)); }

	void setGreen(float green) { m_value.setGreen(Scalar(green)); }

	void setBlue(float blue) { m_value.setBlue(Scalar(blue)); }

	void setAlpha(float alpha) { m_value.setAlpha(Scalar(alpha)); }

	const Color4f& unbox() const { return m_value; }

private:
	Color4f m_value;
};

class T_DLLCLASS BoxedRefArray : public Object
{
	T_RTTI_CLASS;

public:
	BoxedRefArray();

	template < typename ObjectType >
	BoxedRefArray(const RefArray< ObjectType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = arr[i];
	}

	int32_t size() const;

	void set(int32_t index, Object* object);

	Object* get(int32_t index);

	void pushBack(Object* object);

	void popBack();

	Object* front();

	Object* back();

	template < typename ObjectType >
	RefArray< ObjectType > unbox() const
	{
		RefArray< ObjectType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = checked_type_cast< ObjectType*, true >(m_arr[i]);
		return arr;
	}

private:
	RefArray< Object > m_arr;
};

class T_DLLCLASS BoxedStdVector : public Object
{
	T_RTTI_CLASS;

public:
	BoxedStdVector();

	template < typename ItemType >
	BoxedStdVector(const std::vector< ItemType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = CastAny< ItemType >::set(arr[i]);
	}

	int32_t size() const;

	void set(int32_t index, const Any& value);

	Any get(int32_t index);

	template < typename ItemType >
	std::vector< ItemType > unbox() const
	{
		std::vector< ItemType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = CastAny< ItemType >::get(m_arr[i]);
		return arr;
	}

private:
	std::vector< Any > m_arr;
};

template < >
struct CastAny< uint64_t, false >
{
	static Any set(const uint64_t& value) {
		return Any(new BoxedUInt64(value));
	}	
	static uint64_t get(const Any& value) {
		return checked_type_cast< BoxedUInt64*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny< const uint64_t&, false >
{
	static Any set(const uint64_t& value) {
		return Any(new BoxedUInt64(value));
	}	
	static uint64_t get(const Any& value) {
		return checked_type_cast< BoxedUInt64*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Guid, false >
{
	static Any set(const Guid& value) {
		return Any(new BoxedGuid(value));
	}	
	static Guid get(const Any& value) {
		return checked_type_cast< BoxedGuid*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Guid&, false >
{
	static Any set(const Guid& value) {
		return Any(new BoxedGuid(value));
	}	
	static Guid get(const Any& value) {
		return checked_type_cast< BoxedGuid*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Vector2, false >
{
	static Any set(const Vector2& value) {
		return Any(new BoxedVector2(value));
	}	
	static Vector2 get(const Any& value) {
		return checked_type_cast< BoxedVector2*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Vector2&, false >
{
	static Any set(const Vector2& value) {
		return Any(new BoxedVector2(value));
	}	
	static Vector2 get(const Any& value) {
		return checked_type_cast< BoxedVector2*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Vector4, false >
{
    static Any set(const Vector4& value) {
        return Any(new BoxedVector4(value));
    }	
    static Vector4 get(const Any& value) {
        return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Vector4&, false >
{
    static Any set(const Vector4& value) {
        return Any(new BoxedVector4(value));
    }	
    static Vector4 get(const Any& value) {
        return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Quaternion, false >
{
    static Any set(const Quaternion& value) {
        return Any(new BoxedQuaternion(value));
    }
    static Quaternion get(const Any& value) {
        return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Quaternion&, false >
{
    static Any set(const Quaternion& value) {
        return Any(new BoxedQuaternion(value));
    }
    static Quaternion get(const Any& value) {
        return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Transform, false >
{
    static Any set(const Transform& value) {
        return Any(new BoxedTransform(value));
    }
    static Transform get(const Any& value) {
        return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Transform&, false >
{
    static Any set(const Transform& value) {
        return Any(new BoxedTransform(value));
    }
    static Transform get(const Any& value) {
        return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Aabb3, false >
{
	static Any set(const Aabb3& value) {
		return Any(new BoxedAabb3(value));
	}	
	static Aabb3 get(const Any& value) {
		return checked_type_cast< BoxedAabb3*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Aabb3&, false >
{
	static Any set(const Aabb3& value) {
		return Any(new BoxedAabb3(value));
	}	
	static Aabb3 get(const Any& value) {
		return checked_type_cast< BoxedAabb3*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Color4f, false >
{
	static Any set(const Color4f& value) {
		return Any(new BoxedColor4f(value));
	}
	static Color4f get(const Any& value) {
		return checked_type_cast< BoxedColor4f*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Color4f&, false >
{
	static Any set(const Color4f& value) {
		return Any(new BoxedColor4f(value));
	}
	static Color4f get(const Any& value) {
		return checked_type_cast< BoxedColor4f*, false >(value.getObject())->unbox();
	}
};

template < typename InnerType >
struct CastAny < RefArray< InnerType >, false >
{
    static Any set(const RefArray< InnerType >& value) {
        return Any(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedRefArray*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const RefArray< InnerType >&, false >
{
    static Any set(const RefArray< InnerType >& value) {
        return Any(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedRefArray*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < std::vector< InnerType >, false >
{
    static Any set(const std::vector< InnerType >& value) {
        return Any(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedStdVector*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const std::vector< InnerType >&, false >
{
    static Any set(const std::vector< InnerType >& value) {
        return Any(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedStdVector*, false >(value.getObject())->unbox< InnerType >();
    }
};

void T_DLLCLASS registerBoxClasses(class IScriptManager* scriptManager);
	
	}
}

#endif	// traktor_script_Boxes_H
