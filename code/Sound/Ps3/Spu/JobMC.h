#pragma once

#include <cell/spurs/job_descriptor.h>

namespace traktor
{
	namespace sound
	{

struct JobMC
{
	CellSpursJobHeader header;

	struct MixerData
	{
		uintptr_t lsbEA;
		uintptr_t rsbEA;
		uint32_t count;
		uint32_t rcount;
		float factor;
	}
	mixer;

	// Ensure job descriptor is 128 byte(s).
	uint8_t pad[128 - sizeof(CellSpursJobHeader) - sizeof(MixerData)];
};

	}
}
