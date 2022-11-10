/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Const.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace physics
	{

/*! Rigid body state.
 * \ingroup Physics
 */
struct BodyState
{
	BodyState()
	:	m_transform(Transform::identity())
	,	m_linearVelocity(Vector4::zero())
	,	m_angularVelocity(Vector4::zero())
	{
	}

	void setTransform(const Transform& transform) {
		m_transform = transform;
	}

	const Transform& getTransform() const {
		return m_transform;
	}

	void setLinearVelocity(const Vector4& velocity) {
		m_linearVelocity = velocity;
	}

	const Vector4& getLinearVelocity() const {
		return m_linearVelocity;
	}

	void setAngularVelocity(const Vector4& velocity) {
		m_angularVelocity = velocity;
	}

	const Vector4& getAngularVelocity() const {
		return m_angularVelocity;
	}

	BodyState interpolate(const BodyState& stateTarget, const Scalar& interpolate) const
	{
		BodyState state;

		state.m_transform = lerp(m_transform, stateTarget.m_transform, interpolate);

		Scalar lv0 = m_linearVelocity.length();
		Scalar lv1 = stateTarget.m_linearVelocity.length();
		Scalar lv = lerp(lv0, lv1, interpolate);

		if (abs(lv) > FUZZY_EPSILON)
			state.m_linearVelocity = lerp(m_linearVelocity, stateTarget.m_linearVelocity, interpolate).normalized() * lv;
		else
			state.m_linearVelocity = Vector4::zero();

		Scalar av0 = m_angularVelocity.length();
		Scalar av1 = stateTarget.m_angularVelocity.length();
		Scalar av = lerp(av0, av1, interpolate);

		if (abs(av) > FUZZY_EPSILON)
			state.m_angularVelocity = lerp(m_angularVelocity, stateTarget.m_angularVelocity, interpolate).normalized() * av;
		else
			state.m_angularVelocity = Vector4::zero();

		return state;
	}

private:
	Transform m_transform;
	Vector4 m_linearVelocity;
	Vector4 m_angularVelocity;
};

	}
}

