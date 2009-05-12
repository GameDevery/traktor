#ifndef traktor_Matrix33_H
#define traktor_Matrix33_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief 3x3 matrix.
 * \ingroup Core
 */
T_MATH_ALIGN16 class T_DLLCLASS Matrix33
{
public:
	union
	{
		union { float m[9]; };
		union { float e[3][3]; };
		struct
		{
			float e11, e12, e13;
			float e21, e22, e23;
			float e31, e32, e33;
		};
	};

	T_MATH_INLINE Matrix33();

	T_MATH_INLINE Matrix33(const Matrix33& m);

	explicit T_MATH_INLINE Matrix33(const Vector4& r1, const Vector4& r2, const Vector4& r3);

	explicit T_MATH_INLINE Matrix33(float e11, float e12, float e13, float e21, float e22, float e23, float e31, float e32, float e33);

	explicit T_MATH_INLINE Matrix33(const float m[9]);

	static T_MATH_INLINE const Matrix33& zero();

	static T_MATH_INLINE const Matrix33& identity();

	T_MATH_INLINE Vector4 diagonal() const;

	T_MATH_INLINE float determinant() const;

	T_MATH_INLINE Matrix33 transpose() const;

	T_MATH_INLINE Matrix33 inverse() const;

	T_MATH_INLINE Matrix33& operator = (const Matrix33& m);

	T_MATH_INLINE Matrix33& operator *= (const Matrix33& m);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector2 operator * (const Matrix33& m, const Vector2& v);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Vector4 operator * (const Matrix33& m, const Vector4& v);

	/*extern*/ friend T_MATH_INLINE T_DLLCLASS Matrix33 operator * (const Matrix33& lh, const Matrix33& rh);
};

T_MATH_INLINE T_DLLCLASS Matrix33 translate(float x, float y);

T_MATH_INLINE T_DLLCLASS Matrix33 rotate(float angle);

T_MATH_INLINE T_DLLCLASS Matrix33 scale(float x, float y);

}

#if defined(T_MATH_USE_INLINE)
#include "Core/Math/Std/Matrix33.inl"
#endif

#endif	// traktor_Matrix33_H
