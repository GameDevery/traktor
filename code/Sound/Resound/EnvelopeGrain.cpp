/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Sound/IAudioMixer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/EnvelopeGrain.h"

namespace traktor::sound
{
	namespace
	{

const uint32_t c_outputSamplesBlockCount = 8;

struct EnvelopeGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	handle_t m_id;
	float m_parameter;
	float m_lastP;
	RefArray< ISoundBufferCursor > m_cursors;
	float* m_outputSamples[SbcMaxChannelCount];

	EnvelopeGrainCursor()
	{
		m_outputSamples[0] = nullptr;
	}

	virtual ~EnvelopeGrainCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (m_id == id)
			m_parameter = parameter;

		for (auto cursor : m_cursors)
			cursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		for (auto cursor : m_cursors)
			cursor->disableRepeat();
	}

	virtual void reset() override final
	{
		for (auto cursor : m_cursors)
			cursor->reset();
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EnvelopeGrain", EnvelopeGrain, IGrain)

EnvelopeGrain::EnvelopeGrain(handle_t id, const std::vector< Grain >& grains, const float levels[3], float mid, float response)
:	m_id(id)
,	m_grains(grains)
,	m_response(response)
{
	m_envelope.addKey(0.0f, levels[0]);
	m_envelope.addKey( mid, levels[1]);
	m_envelope.addKey(1.0f, levels[2]);
}

Ref< ISoundBufferCursor > EnvelopeGrain::createCursor() const
{
	if (m_grains.empty())
		return nullptr;

	Ref< EnvelopeGrainCursor > cursor = new EnvelopeGrainCursor();
	cursor->m_id = m_id;
	cursor->m_parameter = 0.0f;
	cursor->m_lastP = 0.0f;
	cursor->m_outputSamples[0] = nullptr;

	cursor->m_cursors.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		cursor->m_cursors[i] = m_grains[i].grain->createCursor();
		if (!cursor->m_cursors[i])
			return nullptr;
	}

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	cursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(cursor->m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		cursor->m_outputSamples[i] = cursor->m_outputSamples[0] + outputSamplesCount * i;

	return cursor;
}

void EnvelopeGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	EnvelopeGrainCursor* envelopeCursor = static_cast< EnvelopeGrainCursor* >(cursor);
	for (uint32_t i = 0; i < m_grains.size(); ++i)
		m_grains[i].grain->updateCursor(envelopeCursor->m_cursors[i]);
}

const IGrain* EnvelopeGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void EnvelopeGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const EnvelopeGrainCursor* envelopeCursor = static_cast< const EnvelopeGrainCursor* >(cursor);
	T_ASSERT(envelopeCursor);

	outActiveGrains.push_back(this);

	const float p = envelopeCursor->m_lastP;
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		const float I = m_grains[i].in;
		const float O = m_grains[i].out;
		const float Ie = m_grains[i].easeIn;
		const float Oe = m_grains[i].easeOut;

		if (p < I - Ie)
			continue;
		if (p > O + Oe)
			continue;

		float v = 0.0f;

		if (p < I)
		{
			float f = (p - I + Ie) / Ie;
			v = std::sin(f * PI / 2.0f);
		}
		else if (p > O)
		{
			float f = 1.0f - (p - O) / Oe;
			v = std::sin(f * PI / 2.0f);
		}
		else
			v = 1.0f;

		if (v < FUZZY_EPSILON)
			continue;

		if (m_grains[i].grain)
			m_grains[i].grain->getActiveGrains(envelopeCursor->m_cursors[i], outActiveGrains);
	}
}

bool EnvelopeGrain::getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	EnvelopeGrainCursor* envelopeCursor = static_cast< EnvelopeGrainCursor* >(cursor);
	T_ASSERT(envelopeCursor);

	const float p0 = clamp(m_envelope(envelopeCursor->m_parameter), 0.0f, 1.0f);

	// Filter parameter to reduce intense changes causing clipping noises.
	const float k = clamp(m_response * float(outBlock.samplesCount) / 44050.0f, 0.0f, 1.0f);
	const float p = envelopeCursor->m_lastP * (1.0f - k) + p0 * k;
	envelopeCursor->m_lastP = p;

	bool result = false;

	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		const float I = m_grains[i].in;
		const float O = m_grains[i].out;
		const float Ie = m_grains[i].easeIn;
		const float Oe = m_grains[i].easeOut;

		if (p < I - Ie)
			continue;
		if (p > O + Oe)
			continue;

		float v = 0.0f;

		if (p < I)
		{
			float f = (p - I + Ie) / Ie;
			v = std::sin(f * PI / 2.0f);
		}
		else if (p > O)
		{
			float f = 1.0f - (p - O) / Oe;
			v = std::sin(f * PI / 2.0f);
		}
		else
			v = 1.0f;

		if (v < FUZZY_EPSILON)
			continue;

		SoundBlock soundBlock = { { nullptr }, outBlock.samplesCount, 0, 0 };
		if (m_grains[i].grain->getBlock(
			envelopeCursor->m_cursors[i],
			mixer,
			soundBlock
		))
		{
			outBlock.sampleRate = soundBlock.sampleRate;
			outBlock.samplesCount = max(outBlock.samplesCount, soundBlock.samplesCount);
			outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);

			if (v > 1.0f - FUZZY_EPSILON)
			{
				for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
					outBlock.samples[j] = soundBlock.samples[j];
			}
			else if (v > FUZZY_EPSILON)
			{
				for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
				{
					if (!soundBlock.samples[j])
						continue;

					if (!outBlock.samples[j])
					{
						outBlock.samples[j] = envelopeCursor->m_outputSamples[j];
						mixer->mulConst(
							outBlock.samples[j],
							soundBlock.samples[j],
							soundBlock.samplesCount,
							v
						);
					}
					else
					{
						mixer->addMulConst(
							outBlock.samples[j],
							soundBlock.samples[j],
							soundBlock.samplesCount,
							v
						);
					}
				}
			}

			result = true;
		}
	}

	return result;
}

}
