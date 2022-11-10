/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Range.h"
#include "Spray/Source.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! Cone particle source.
 * \ingroup Spray
 */
class T_DLLCLASS ConeSource : public Source
{
	T_RTTI_CLASS;

public:
	ConeSource(
		float constantRate,
		float velocityRate,
		const Vector4& position,
		const Vector4& normal,
		float angle1,
		float angle2,
		const Range< float >& velocity,
		const Range< float >& inheritVelocity,
		const Range< float >& orientation,
		const Range< float >& angularVelocity,
		const Range< float >& age,
		const Range< float >& mass,
		const Range< float >& size
	);

	virtual void emit(
		Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const override final;

	const Vector4& getPosition() const { return m_position; }

	const Vector4& getNormal() const { return m_normal; }

	const Scalar& getAngle1s() const { return m_angle1s; }

	const Scalar& getAngle2s() const { return m_angle2s; }

private:
	Vector4 m_position;
	Vector4 m_normal;
	Scalar m_angle1s;
	Scalar m_angle2s;
	Range< float > m_velocity;
	Range< float > m_inheritVelocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

