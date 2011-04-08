#include <cstring>
#include <algorithm>
#include "Core/Io/BufferedStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.BufferedStream", BufferedStream, IStream)

BufferedStream::BufferedStream(IStream* stream, uint32_t internalBufferSize)
:	m_stream(stream)
,	m_internalBufferSize(internalBufferSize)
,	m_readBuf(0)
,	m_writeBuf(0)
{
	m_readBufCnt[0] =
	m_readBufCnt[1] = 0;
	m_writeBufCnt = 0;

	if (m_stream->canRead())
		m_readBuf = new uint8_t [m_internalBufferSize];
	if (m_stream->canWrite())
		m_writeBuf = new uint8_t [m_internalBufferSize];
}

BufferedStream::BufferedStream(IStream* stream, const void* appendData, uint32_t appendDataSize, uint32_t internalBufferSize)
:	m_stream(stream)
,	m_internalBufferSize(internalBufferSize)
,	m_readBuf(0)
,	m_writeBuf(0)
{
	T_ASSERT (appendData);
	T_ASSERT (appendDataSize <= internalBufferSize);

	m_readBufCnt[0] =
	m_readBufCnt[1] = 0;
	m_writeBufCnt = 0;

	if (m_stream->canRead())
	{
		m_readBuf = new uint8_t [m_internalBufferSize];
		std::memcpy(m_readBuf, appendData, appendDataSize);
		m_readBufCnt[1] = appendDataSize;
	}
	if (m_stream->canWrite())
	{
		m_writeBuf = new uint8_t [m_internalBufferSize];
		std::memcpy(m_writeBuf, appendData, appendDataSize);
		m_writeBufCnt = appendDataSize;
	}
}

BufferedStream::~BufferedStream()
{
	delete[] m_writeBuf;
	delete[] m_readBuf;
}

void BufferedStream::close()
{
	if (m_stream)
	{
		flush();
		m_stream->close();
		m_stream = 0;
	}
}

bool BufferedStream::canRead() const
{
	return m_stream->canRead();
}

bool BufferedStream::canWrite() const
{
	return m_stream->canWrite();
}

bool BufferedStream::canSeek() const
{
	return m_stream->canSeek();
}

int BufferedStream::tell() const
{
	if (m_stream->canRead())
		return m_stream->tell() + m_readBufCnt[0];
	if (m_stream->canWrite())
		return m_stream->tell() + m_writeBufCnt;
	return m_stream->tell();
}

int BufferedStream::available() const
{
	if (m_stream->canRead())
		return m_stream->available() + (m_readBufCnt[1] - m_readBufCnt[0]);
	else
		return m_stream->available();
}

int BufferedStream::seek(SeekOriginType origin, int offset)
{
	if (m_stream->canRead())
	{
		int32_t readBufAvail = m_readBufCnt[1] - m_readBufCnt[0];
		switch (origin)
		{
		case SeekSet:
			offset -= tell();
			// Fall through

		case SeekCurrent:
			{
				if (offset > 0)
				{
					if (offset < readBufAvail)
					{
						m_readBufCnt[0] += offset;
						return offset;
					}
					else
					{
						m_readBufCnt[0] =
						m_readBufCnt[1] = 0;
						return m_stream->seek(SeekCurrent, offset - readBufAvail);
					}
				}
				else if (offset < 0)
				{
					int32_t position = tell() + offset;
					m_readBufCnt[0] =
					m_readBufCnt[1] = 0;
					return m_stream->seek(SeekSet, position);
				}
				else
					return 0;
			}
			break;

		case SeekEnd:
			m_readBufCnt[0] =
			m_readBufCnt[1] = 0;
			return m_stream->seek(SeekEnd, offset);
		}

		return -1;
	}
	else if (m_stream->canWrite())
		flush();

	return m_stream->seek(origin, offset);
}

int BufferedStream::read(void* block, int nbytes)
{
	uint8_t* out = static_cast< uint8_t* >(block);
	uint8_t* end = out + nbytes;

	if (nbytes <= int(m_internalBufferSize))
	{
		// Read and copy until number of desired bytes read is meet.
		while (out < end)
		{
			if (m_readBufCnt[0] >= m_readBufCnt[1])
			{
				// Read into buffer.
				int nread = m_stream->read(m_readBuf, m_internalBufferSize);
				if (nread <= 0)
					break;
				m_readBufCnt[0] = 0;
				m_readBufCnt[1] = nread;
			}

			// Copy from read buffer into output buffer.
			int ncopy = std::min(int(end - out), m_readBufCnt[1] - m_readBufCnt[0]);
			T_ASSERT (ncopy > 0);

			std::memcpy(out, &m_readBuf[m_readBufCnt[0]], ncopy);

			m_readBufCnt[0] += ncopy;
			T_ASSERT (m_readBufCnt[0] <= m_readBufCnt[1]);

			out += ncopy;
		}
	}
	else
	{
		// Requested more bytes than our internal buffer, read directly from stream.
		if (m_readBufCnt[0] < m_readBufCnt[1])
		{
			int ncopy = m_readBufCnt[1] - m_readBufCnt[0];
			T_ASSERT (ncopy <= nbytes);

			std::memcpy(out, &m_readBuf[m_readBufCnt[0]], ncopy);

			m_readBufCnt[0] += ncopy;
			T_ASSERT (m_readBufCnt[0] <= m_readBufCnt[1]);

			out += ncopy;
		}

		int nread = m_stream->read(out, int(end - out));
		if (nread > 0)
			out += nread;
		else if (nread < 0 && out == block)
			return -1;
	}

	// Returned number of bytes copied.
	return static_cast< int >(out - static_cast< uint8_t* >(block));
}

int BufferedStream::write(const void* block, int nbytes)
{
	int nwritten = 0;

	if (nbytes < int(m_internalBufferSize))
	{
		const uint8_t* ptr = static_cast< const uint8_t* >(block);
		while (nbytes > 0)
		{
			int space = m_internalBufferSize - m_writeBufCnt;
			int nwrite = std::min< int >(space, nbytes);
			
			std::memcpy(&m_writeBuf[m_writeBufCnt], ptr, nwrite);
			
			m_writeBufCnt += nwrite;
			nwritten += nwrite;
			nbytes -= nwrite;
			ptr += nwrite;

			if (m_writeBufCnt >= int(m_internalBufferSize))
				flush();
		}
	}
	else
	{
		flush();
		nwritten = m_stream->write(block, nbytes);
	}

	return nwritten;
}

void BufferedStream::flush()
{
	T_ASSERT (m_stream);
	if (m_writeBufCnt > 0)
	{
		m_stream->write(m_writeBuf, m_writeBufCnt);
		m_writeBufCnt = 0;
	}
	m_stream->flush();
}

}
