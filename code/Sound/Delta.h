#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BitReader;
class BitWriter;

	namespace sound
	{

class T_DLLCLASS Delta
{
public:
	/*! Encode signed 16-bit sample data with delta encoding.
	* \ingroup Sound
	*
	* \param data Input raw data
	* \param count Number of samples in raw data.
	* \param bw BitWriter output.
	* \return Number of bytes written.
	*/
	static uint32_t encode(const int16_t* data, uint32_t count, BitWriter& bw);

	/*! Decode delta encoded block into 16-bit signed data.
	* \ingroup Sound
	*/
	static void decode(BitReader& br, uint32_t count, int16_t* out);
};

	}
}

