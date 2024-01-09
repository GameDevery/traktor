/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/LineSource.h"
#include "Spray/Sources/LineSourceData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.LineSourceData", 0, LineSourceData, SourceData)

LineSourceData::LineSourceData()
:	SourceData()
,	m_startPosition(0.0f, 0.0f, 0.0f, 1.0f)
,	m_endPosition(0.0f, 0.0f, 0.0f, 1.0f)
,	m_segments(0)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

Ref< const Source > LineSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new LineSource(
		getConstantRate(),
		getVelocityRate(),
		m_startPosition,
		m_endPosition,
		m_segments,
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void LineSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> Member< Vector4 >(L"startPosition", m_startPosition, AttributePoint());
	s >> Member< Vector4 >(L"endPosition", m_endPosition, AttributePoint());
	s >> Member< int32_t >(L"segments", m_segments, AttributeRange(0));
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

}
