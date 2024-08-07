/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/HingeJointDesc.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HingeJointDesc", 2, HingeJointDesc, JointDesc)

HingeJointDesc::HingeJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis(1.0f, 0.0f, 0.0f, 0.0f)
,	m_enableLimits(false)
,	m_minAngle(0.0f)
,	m_maxAngle(0.0f)
,	m_angularOnly(false)
{
}

void HingeJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& HingeJointDesc::getAnchor() const
{
	return m_anchor;
}

void HingeJointDesc::setAxis(const Vector4& axis)
{
	m_axis = axis;
}

const Vector4& HingeJointDesc::getAxis() const
{
	return m_axis;
}

void HingeJointDesc::setEnableLimits(bool enableLimits)
{
	m_enableLimits = enableLimits;
}

bool HingeJointDesc::getEnableLimits() const
{
	return m_enableLimits;
}

void HingeJointDesc::setAngles(float minAngle, float maxAngle)
{
	m_minAngle = minAngle;
	m_maxAngle = maxAngle;
}

void HingeJointDesc::getAngles(float& outMinAngle, float& outMaxAngle) const
{
	outMinAngle = m_minAngle;
	outMaxAngle = m_maxAngle;
}

void HingeJointDesc::setAngularOnly(bool angularOnly)
{
	m_angularOnly = angularOnly;
}

bool HingeJointDesc::getAngularOnly() const
{
	return m_angularOnly;
}

void HingeJointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"enableLimits", m_enableLimits);

	s >> Member< float >(L"minAngle", m_minAngle, AttributeRange(-PI, PI));
	s >> Member< float >(L"maxAngle", m_maxAngle, AttributeRange(-PI, PI));

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"angularOnly", m_angularOnly);
}

}
