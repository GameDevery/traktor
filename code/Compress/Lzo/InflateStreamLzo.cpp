#include <minilzo.h>
#include "Compress/Lzo/InflateStreamLzo.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Reader.h"

namespace traktor
{
	namespace compress
	{

class InflateLzoImpl : public RefCountImpl< IRefCount >
{
public:
	InflateLzoImpl(IStream* stream, uint32_t blockSize)
	:	m_stream(stream)
	,	m_compressedBlock((blockSize + blockSize / 16 + 64 + 3))
	,	m_decompressedBuffer(blockSize)
	,	m_decompressedBufferSize(0)
	{
	}

	void close()
	{
		m_stream = 0;
	}

	int read(void* block, int nbytes)
	{
		uint8_t* top = static_cast< uint8_t* >(block);
		uint8_t* ptr = static_cast< uint8_t* >(block);

		// Copy from buffer.
		if (m_decompressedBufferSize > 0)
		{
			int32_t ncopy = std::min< int32_t >(m_decompressedBufferSize, nbytes);
			
			std::memcpy(ptr, &m_decompressedBuffer[0], ncopy);
			ptr += ncopy;

			if (ncopy < m_decompressedBufferSize)
				std::memmove(&m_decompressedBuffer[0], &m_decompressedBuffer[ncopy], m_decompressedBufferSize - ncopy);

			m_decompressedBufferSize -= ncopy;
			nbytes -= ncopy;
		}

		if (nbytes <= 0)
			return int32_t(ptr - top);

		T_ASSERT (m_decompressedBufferSize == 0);

		// Decompress directly into destination if requested block size is larger than buffer;
		// and output is large enough to contain a bit of overhead.
		while (nbytes >= int(m_decompressedBuffer.size()))
		{
			uint32_t compressedBlockSize = 0;
			Reader(m_stream) >> compressedBlockSize;
			
			if (!compressedBlockSize || compressedBlockSize > m_compressedBlock.size())
				break;

			if (m_stream->read(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
				break;

			lzo_uint decompressLength = m_decompressedBuffer.size();

			int r = lzo1x_decompress_safe(
				&m_compressedBlock[0],
				compressedBlockSize,
				ptr,
				&decompressLength,
				0
			);
			if (r != LZO_E_OK)
				break;

			ptr += decompressLength;
			nbytes -= decompressLength;
		}

		// Decompress into buffer and copy required size into output.
		while (nbytes > 0)
		{
			uint32_t compressedBlockSize = 0;
			Reader(m_stream) >> compressedBlockSize;

			if (!compressedBlockSize || compressedBlockSize > m_compressedBlock.size())
				break;

			if (m_stream->read(&m_compressedBlock[0], compressedBlockSize) != compressedBlockSize)
				break;

			T_ASSERT (m_decompressedBufferSize == 0);
			m_decompressedBufferSize = m_decompressedBuffer.size();

			int r = lzo1x_decompress_safe(
				&m_compressedBlock[0],
				compressedBlockSize,
				&m_decompressedBuffer[0],
				(lzo_uint*)&m_decompressedBufferSize,
				0
			);
			if (r != LZO_E_OK || !m_decompressedBufferSize)
				break;

			int32_t ncopy = std::min< int32_t >(m_decompressedBufferSize, nbytes);
			
			std::memcpy(ptr, &m_decompressedBuffer[0], ncopy);
			ptr += ncopy;

			if (ncopy < m_decompressedBufferSize)
				std::memmove(&m_decompressedBuffer[0], &m_decompressedBuffer[ncopy], m_decompressedBufferSize - ncopy);

			m_decompressedBufferSize -= ncopy;
			nbytes -= ncopy;
		}

		return int32_t(ptr - top);
	}

private:
	Ref< IStream > m_stream;
	AlignedVector< uint8_t > m_compressedBlock;
	AlignedVector< uint8_t > m_decompressedBuffer;
	int32_t m_decompressedBufferSize;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.InflateStreamLzo", InflateStreamLzo, IStream)

InflateStreamLzo::InflateStreamLzo(IStream* stream, uint32_t blockSize)
:	m_impl(new InflateLzoImpl(stream, blockSize))
{
}

InflateStreamLzo::~InflateStreamLzo()
{
	close();
}

void InflateStreamLzo::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool InflateStreamLzo::canRead() const
{
	return true;
}

bool InflateStreamLzo::canWrite() const
{
	return false;
}

bool InflateStreamLzo::canSeek() const
{
	return false;
}

int InflateStreamLzo::tell() const
{
	T_FATAL_ERROR;
	return 0;
}

int InflateStreamLzo::available() const
{
	T_FATAL_ERROR;
	return 0;
}

int InflateStreamLzo::seek(SeekOriginType origin, int offset)
{
	T_FATAL_ERROR;
	return 0;
}

int InflateStreamLzo::read(void* block, int nbytes)
{
	return m_impl->read(block, nbytes);
}

int InflateStreamLzo::write(const void* block, int nbytes)
{
	T_FATAL_ERROR;
	return 0;
}

void InflateStreamLzo::flush()
{
}

	}
}
