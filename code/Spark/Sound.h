#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! Sound container.
 * \ingroup Spark
 */
class T_DLLCLASS Sound : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool create(uint8_t channels, uint32_t sampleRate, uint32_t sampleCount);

	uint8_t getChannels() const { return m_channels; }

	uint32_t getSampleRate() const { return m_sampleRate; }

	uint32_t getSampleCount() const { return m_sampleCount; }

	int16_t* getSamples(uint8_t channel) { return m_samples[channel].ptr(); }

	const int16_t* getSamples(uint8_t channel) const { return m_samples[channel].c_ptr(); }

	virtual void serialize(ISerializer& s) override final;

private:
	uint8_t m_channels = 0;
	uint32_t m_sampleRate = 0;
	uint32_t m_sampleCount = 0;
	AutoArrayPtr< int16_t > m_samples[2];
};

	}
}

