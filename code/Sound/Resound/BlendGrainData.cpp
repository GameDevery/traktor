/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Resound/BlendGrain.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/IGrainFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BlendGrainData", 2, BlendGrainData, IGrainData)

BlendGrainData::BlendGrainData()
:	m_response(1.0f)
{
}

Ref< IGrain > BlendGrainData::createInstance(IGrainFactory* grainFactory) const
{
	Ref< IGrain > grains[2];

	grains[0] = grainFactory->createInstance(m_grains[0]);
	if (!grains[0])
		return 0;

	grains[1] = grainFactory->createInstance(m_grains[1]);
	if (!grains[1])
		return 0;

	return new BlendGrain(
		getParameterHandle(m_id),
		m_response,
		grains[0],
		grains[1]
	);
}

void BlendGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"id", m_id);

	if (s.getVersion() >= 2)
		s >> Member< float >(L"response", m_response);

	s >> MemberStaticArray< Ref< IGrainData >, 2, MemberRef< IGrainData > >(L"grains", m_grains);
}

	}
}
