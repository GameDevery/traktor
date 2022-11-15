/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 4 channel, 32-bit, color.
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Color4f
{
public:
	T_MATH_INLINE Color4f() = default;

	T_MATH_INLINE Color4f(const Color4f &src);

	T_MATH_INLINE Color4f(float red, float green, float blue, float alpha = 0.0f);

	T_MATH_INLINE explicit Color4f(const float *data);

	T_MATH_INLINE explicit Color4f(const Vector4 &data);

	T_MATH_INLINE Scalar get(int channel) const;

	T_MATH_INLINE Scalar getRed() const;

	T_MATH_INLINE Scalar getGreen() const;

	T_MATH_INLINE Scalar getBlue() const;

	T_MATH_INLINE Scalar getAlpha() const;

	T_MATH_INLINE void set(int channel, const Scalar &value);

	T_MATH_INLINE void set(float red, float green, float blue, float alpha = 0.0f);

	T_MATH_INLINE void setRed(const Scalar& red);

	T_MATH_INLINE void setGreen(const Scalar &green);

	T_MATH_INLINE void setBlue(const Scalar &blue);

	T_MATH_INLINE void setAlpha(const Scalar &alpha);

	T_MATH_INLINE Scalar getEV() const;

	T_MATH_INLINE void setEV(const Scalar& ev);

	T_MATH_INLINE Color4f sRGB() const;

	T_MATH_INLINE Color4f linear() const;

	T_MATH_INLINE Color4f saturated() const;

	T_MATH_INLINE Color4f rgb0() const;

	T_MATH_INLINE Color4f rgb1() const;

	T_MATH_INLINE Color4f aaa0() const;

	T_MATH_INLINE Color4f aaa1() const;

	T_MATH_INLINE Color4f aaaa() const;

	T_MATH_INLINE Color4ub toColor4ub() const;

	static T_MATH_INLINE Color4f loadAligned(const float *in);

	static T_MATH_INLINE Color4f loadUnaligned(const float *in);

	static T_MATH_INLINE Color4f fromColor4ub(const Color4ub& in);

	T_MATH_INLINE void storeAligned(float *out) const;

	T_MATH_INLINE void storeUnaligned(float *out) const;

	T_MATH_INLINE Color4f& operator = (const Color4f &src);

	T_MATH_INLINE Color4f operator + (const Color4f &r) const;

	T_MATH_INLINE Color4f operator - (const Color4f &r) const;

	T_MATH_INLINE Color4f operator * (const Color4f &r) const;

	T_MATH_INLINE Color4f operator * (const Scalar& r) const;

	T_MATH_INLINE Color4f operator / (const Color4f &r) const;

	T_MATH_INLINE Color4f operator / (const Scalar& r) const;

	T_MATH_INLINE Color4f& operator += (const Color4f &r);

	T_MATH_INLINE Color4f& operator -= (const Color4f &r);

	T_MATH_INLINE Color4f& operator *= (const Color4f &r);

	T_MATH_INLINE Color4f& operator *= (const Scalar& r);

	T_MATH_INLINE Color4f& operator /= (const Color4f& r);

	T_MATH_INLINE Color4f& operator /= (const Scalar& r);

	T_MATH_INLINE bool operator == (const Color4f &r) const;

	T_MATH_INLINE bool operator != (const Color4f &r) const;

	T_MATH_INLINE operator const Vector4 &() const;

private:
	Vector4 m_data = Vector4::zero();
};

#if defined(min)
#	undef min
#endif
T_MATH_INLINE T_DLLCLASS Color4f min(const Color4f &l, const Color4f &r);

#if defined(max)
#	undef max
#endif
T_MATH_INLINE T_DLLCLASS Color4f max(const Color4f &l, const Color4f &r);

} // namespace traktor

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Color4f.inl"
#endif
