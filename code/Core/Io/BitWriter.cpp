#include "Core/Io/BitWriter.h"
#include "Core/Io/IStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.BitWriter", BitWriter, Object)

BitWriter::BitWriter(IStream* stream)
:	m_stream(stream)
,	m_data(0)
,	m_cnt(0)
{
}

BitWriter::~BitWriter()
{
	flush();
}

void BitWriter::writeBit(bool bit)
{
	if (bit)
		m_data |= 1 << (7 - m_cnt);
	else
		m_data &= ~(1 << (7 - m_cnt));

	if (++m_cnt >= 8)
		flush();
}

void BitWriter::writeUnsigned(int32_t nbits, uint32_t value)
{
	for (int32_t i = 0; i < nbits; ++i)
	{
		uint32_t bit = 1 << (nbits - i - 1);
		writeBit((value & bit) == bit);
	}
}

void BitWriter::writeSigned(int32_t nbits, int32_t value)
{
	writeUnsigned(nbits, *(uint32_t*)&value);
}

void BitWriter::flush()
{
	if (m_cnt <= 0)
		return;

	m_stream->write(&m_data, 1);
	m_cnt = 0;
}

uint32_t BitWriter::tell() const
{
	return (m_stream->tell() << 3) + m_cnt;
}

Ref< IStream > BitWriter::getStream()
{
	return m_stream;
}

}
