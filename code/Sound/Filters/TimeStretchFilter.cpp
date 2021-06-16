#include <cstring>
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/Align.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Thread/Acquire.h"
#include "Sound/Filters/TimeStretchFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct TimeStretchFilterInstance : public RefCountImpl< IFilterInstance >
{
	float* m_samples[SbcMaxChannelCount];

	TimeStretchFilterInstance()
	{
		for (int32_t i = 0; i < SbcMaxChannelCount; ++i)
			m_samples[i] = (float*)Alloc::acquireAlign(128000 * sizeof(float), 16, T_FILE_LINE);
	}

	virtual ~TimeStretchFilterInstance()
	{
		for (int32_t i = 0; i < SbcMaxChannelCount; ++i)
			Alloc::freeAlign(m_samples[i]);
	}

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.TimeStretchFilter", 0, TimeStretchFilter, IFilter)

TimeStretchFilter::TimeStretchFilter(float factor)
:	m_factor(factor)
{
}

Ref< IFilterInstance > TimeStretchFilter::createInstance() const
{
	return new TimeStretchFilterInstance();
}

void TimeStretchFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	TimeStretchFilterInstance* fi = static_cast< TimeStretchFilterInstance* >(instance);

	const int32_t outputSamplesCount = (int32_t)(m_factor * outBlock.samplesCount);

	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		const float* sourceSamples = outBlock.samples[i];
		if (!sourceSamples)
			continue;

		float* outputSamples = fi->m_samples[i];

		for (int32_t j = 0; j < outputSamplesCount; ++j)
		{
			int32_t idx = (int32_t)(j / m_factor);
			float s = sourceSamples[idx];
			outputSamples[j] = s;
		}

		outBlock.samples[i] = outputSamples;
		outBlock.samplesCount = outputSamplesCount;
	}
}

void TimeStretchFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"factor", m_factor);
}

	}
}
