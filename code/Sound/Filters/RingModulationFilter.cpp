/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/RingModulationFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct RingModulationFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	float m_time;

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RingModulationFilter", 0, RingModulationFilter, IAudioFilter)

RingModulationFilter::RingModulationFilter(uint32_t sampleRate, uint32_t ringFrequency)
:	m_sampleRate(sampleRate)
,	m_ringFrequency(ringFrequency)
{
}

Ref< IAudioFilterInstance > RingModulationFilter::createInstance() const
{
	Ref< RingModulationFilterInstance > rmfi = new RingModulationFilterInstance();
	rmfi->m_time = 0.0f;
	return rmfi;
}

void RingModulationFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	RingModulationFilterInstance* rmfi = static_cast< RingModulationFilterInstance* >(instance);
	const float sampleDeltaTime = 1.0f / m_sampleRate;

	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float ringAmplitude = sinf(m_ringFrequency * rmfi->m_time * 2.0f * PI);
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			outBlock.samples[j][i] *= ringAmplitude;
		rmfi->m_time += sampleDeltaTime;
	}
}

void RingModulationFilter::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"ringFrequency", m_ringFrequency);
}

	}
}
