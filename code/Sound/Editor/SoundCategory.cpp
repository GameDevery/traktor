/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Types.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundCategory", 5, SoundCategory, ISerializable)


SoundCategory::SoundCategory()
:	m_gain(0.0f)
,	m_presence(0.0f)
,	m_presenceRate(0.25f)
,	m_range(0.0f)
{
}

void SoundCategory::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"parent", m_parent, AttributeType(type_of< SoundCategory >()));

	if (s.getVersion() >= 4)
		s >> Member< std::wstring >(L"configurationId", m_configurationId);

	if (s.getVersion() >= 5)
		s >> Member< float >(L"gain", m_gain, AttributeUnit(UnitType::Decibel));
	else
	{
		float volumeLin = 1.0f;
		s >> Member< float >(L"volume", volumeLin);
		m_gain = linearToDecibel(volumeLin);
	}

	if (s.getVersion() >= 1)
		s >> Member< float >(L"presence", m_presence, AttributeRange(0.0f));

	if (s.getVersion() >= 2)
		s >> Member< float >(L"presenceRate", m_presenceRate, AttributeRange(0.0f));

	if (s.getVersion() >= 3)
		s >> Member< float >(L"range", m_range, AttributeRange(0.0f));
}

	}
}
