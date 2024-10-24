/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/EmitterInstanceCPU.h"
#include "Spray/Types.h"
#include "Spray/Sources/DiscSource.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.DiscSource", DiscSource, Source)

DiscSource::DiscSource(
	float constantRate,
	float velocityRate,
	const Vector4& position,
	const Vector4& normal,
	const Range< float >& radius,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_position(position.xyz1())
,	m_normal(normal.xyz0())
,	m_radius(radius)
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void DiscSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	const Vector4 axisZ(0.0f, 0.0f, 1.0f, 0.0f);

	const Vector4 wx = cross(m_normal, axisZ);
	const Vector4 wz = cross(wx, m_normal);

	const Vector4 position = transform * m_position;
	const Vector4 x = transform * wx;
	const Vector4 y = transform * m_normal;
	const Vector4 z = transform * wz;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		const Vector4 direction = (x * Scalar(context.random.nextFloat() * 2.0f - 1.0f) + z * Scalar(context.random.nextFloat() * 2.0f - 1.0f)).normalized();

		point->position = position + direction * Scalar(m_radius.random(context.random));
		point->velocity = y * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();

		++point;
	}
}

}
