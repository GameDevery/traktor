#include <algorithm>
#include <cstring>
#include <zlib.h>
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"

namespace traktor
{
	namespace compress
	{
		namespace
		{

voidpf inflateZAlloc(voidpf opaque, uInt items, uInt size)
{
	return Alloc::acquire(items * size, "zlib");
}

void inflateZFree(voidpf opaque, voidpf address)
{
	Alloc::free(address);
}

		}

class InflateZipImpl : public RefCountImpl< IRefCount >
{
public:
	InflateZipImpl(IStream* stream, uint32_t internalBufferSize, bool negativeWindowBits)
	:	m_stream(stream)
	,	m_buf(internalBufferSize)
	,	m_startPosition(stream->tell())
	,	m_position(m_startPosition)
	{
		std::memset(&m_zstream, 0, sizeof(m_zstream));

		m_zstream.zalloc = &inflateZAlloc;
		m_zstream.zfree = &inflateZFree;

		int rc;
		if (!negativeWindowBits)
			rc = inflateInit(&m_zstream);
		else
			rc = inflateInit2(&m_zstream, -MAX_WBITS);

		T_FATAL_ASSERT (rc == Z_OK);
	}

	void close()
	{
		inflateEnd(&m_zstream);
		m_stream = nullptr;
	}

	int64_t read(void* block, int64_t nbytes)
	{
		int rc;

		m_zstream.next_out = (Bytef*)block;
		m_zstream.avail_out = (uInt)nbytes;

		for (;;)
		{
			if (m_zstream.avail_in == 0)
			{
				int64_t nread = m_stream->read(&m_buf[0], (int64_t)m_buf.size());
				if (!nread)
					break;

				m_zstream.next_in = &m_buf[0];
				m_zstream.avail_in = nread;
			}

			rc = inflate(&m_zstream, Z_SYNC_FLUSH);
			if (rc != Z_OK || m_zstream.avail_out == 0)
				break;
		}

		int64_t nread = nbytes - m_zstream.avail_out;

		m_position += nread;
		return nread;
	}

	int64_t setLogicalPosition(int64_t position)
	{
		// Seeking backwards, restart from beginning.
		if (position < m_position)
		{
			inflateReset(&m_zstream);
			m_stream->seek(IStream::SeekSet, m_startPosition);
			m_zstream.next_in = 0;
			m_zstream.avail_in = 0;
			m_position = m_startPosition;
		}

		// Read dummy blocks until we're at the desired position.
		uint8_t dummy[1024];
		while (m_position < position)
		{
			int64_t nread = read(dummy, std::min< int64_t >(sizeof_array(dummy), position - m_position));
			if (nread <=  0)
				return -1;
		}

		return m_position;
	}

	int64_t getLogicalPosition() const
	{
		return m_position;
	}

private:
	Ref< IStream > m_stream;
	z_stream m_zstream;
	AlignedVector< uint8_t > m_buf;
	int64_t m_startPosition;
	int64_t m_position;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.InflateStreamZip", InflateStreamZip, IStream)

InflateStreamZip::InflateStreamZip(IStream* stream, uint32_t internalBufferSize, bool negativeWindowBits)
:	m_impl(new InflateZipImpl(stream, internalBufferSize, negativeWindowBits))
{
}

InflateStreamZip::~InflateStreamZip()
{
	close();
}

void InflateStreamZip::close()
{
	safeClose(m_impl);
}

bool InflateStreamZip::canRead() const
{
	return true;
}

bool InflateStreamZip::canWrite() const
{
	return false;
}

bool InflateStreamZip::canSeek() const
{
	return true;
}

int64_t InflateStreamZip::tell() const
{
	return m_impl->getLogicalPosition();
}

int64_t InflateStreamZip::available() const
{
	T_ASSERT(0);
	return 0;
}

int64_t InflateStreamZip::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT_M (origin != SeekEnd, L"Only SeekEnd is allowed on InflateStreamZip");
	if (origin == SeekCurrent)
		offset += m_impl->getLogicalPosition();
	return m_impl->setLogicalPosition(offset);
}

int64_t InflateStreamZip::read(void* block, int64_t nbytes)
{
	return m_impl->read(block, nbytes);
}

int64_t InflateStreamZip::write(const void* block, int64_t nbytes)
{
	T_ASSERT(0);
	return 0;
}

void InflateStreamZip::flush()
{
}

	}
}
