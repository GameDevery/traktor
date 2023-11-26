/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Resound/EnvelopeGrain.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/IGrainFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.EnvelopeGrainData", 3, EnvelopeGrainData, IGrainData)

EnvelopeGrainData::EnvelopeGrainData()
:	m_mid(0.5f)
,	m_response(1.0f)
{
	m_levels[0] = 0.0f;
	m_levels[1] = 0.5f;
	m_levels[2] = 1.0f;
}

void EnvelopeGrainData::addGrain(IGrainData* grain, float in, float out, float easeIn, float easeOut)
{
	GrainData gd;
	gd.grain = grain;
	gd.in = in;
	gd.out = out;
	gd.easeIn = easeIn;
	gd.easeOut = easeOut;
	m_grains.push_back(gd);
}

void EnvelopeGrainData::removeGrain(IGrainData* grain)
{
	T_FATAL_ERROR;
}

void EnvelopeGrainData::setLevels(const float levels[3])
{
	m_levels[0] = levels[0];
	m_levels[1] = levels[1];
	m_levels[2] = levels[2];
}

void EnvelopeGrainData::setMid(float mid)
{
	m_mid = mid;
}

void EnvelopeGrainData::setResponse(float response)
{
	m_response = response;
}

Ref< IGrain > EnvelopeGrainData::createInstance(IGrainFactory* grainFactory) const
{
	AlignedVector< EnvelopeGrain::Grain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i].grain = grainFactory->createInstance(m_grains[i].grain);
		if (!grains[i].grain)
			return 0;

		grains[i].in = m_grains[i].in;
		grains[i].out = m_grains[i].out;
		grains[i].easeIn = m_grains[i].easeIn;
		grains[i].easeOut = m_grains[i].easeOut;
	}

	return new EnvelopeGrain(
		getParameterHandle(m_id),
		grains,
		m_levels,
		m_mid,
		m_response
	);
}

void EnvelopeGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"id", m_id);

	s >> MemberAlignedVector< GrainData, MemberComposite< GrainData > >(L"grains", m_grains, AttributePrivate());

	if (s.getVersion() >= 2)
	{
		s >> MemberStaticArray< float, sizeof_array(m_levels) >(L"levels", m_levels, AttributePrivate());
		s >> Member< float >(L"mid", m_mid, AttributePrivate());
	}

	if (s.getVersion() >= 3)
		s >> Member< float >(L"response", m_response);
}

void EnvelopeGrainData::GrainData::serialize(ISerializer& s)
{
	s >> MemberRef< IGrainData >(L"grain", grain);
	s >> Member< float >(L"in", in);
	s >> Member< float >(L"out", out);
	s >> Member< float >(L"easeIn", easeIn);
	s >> Member< float >(L"easeOut", easeOut);
}

}
