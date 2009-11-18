#include <cstring>
#include <sstream>
#include "Editor/MemCachedPutStream.h"
#include "Editor/MemCachedProto.h"
#include "Core/Thread/Acquire.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPutStream", MemCachedPutStream, IStream)

MemCachedPutStream::MemCachedPutStream(MemCachedProto* proto, const std::string& key)
:	m_proto(proto)
,	m_key(key)
,	m_inblock(0)
,	m_index(0)
{
}

void MemCachedPutStream::close()
{
	flush();
	m_proto = 0;
}

bool MemCachedPutStream::canRead() const
{
	return false;
}

bool MemCachedPutStream::canWrite() const
{
	return true;
}

bool MemCachedPutStream::canSeek() const
{
	return false;
}

int MemCachedPutStream::tell() const
{
	return 0;
}

int MemCachedPutStream::available() const
{
	return 0;
}

int MemCachedPutStream::seek(SeekOriginType origin, int offset)
{
	return 0;
}

int MemCachedPutStream::read(void* block, int nbytes)
{
	return 0;
}

int MemCachedPutStream::write(const void* block, int nbytes)
{
	const uint8_t* blockPtr = static_cast< const uint8_t* >(block);
	int written = 0;

	while (written < nbytes)
	{
		int avail = MaxBlockSize - m_inblock;
		int copy = std::min(nbytes - written, avail);

		std::memcpy(&m_block[m_inblock], blockPtr, copy);

		blockPtr += copy;
		m_inblock += copy;
		written += copy;

		if (m_inblock >= MaxBlockSize)
		{
			if (uploadBlock())
				m_inblock = 0;
			else
				break;
		}
	}

	return written;
}

void MemCachedPutStream::flush()
{
	if (m_inblock > 0)
		uploadBlock();
}

bool MemCachedPutStream::uploadBlock()
{
	Acquire< Semaphore > lock(m_proto->getLock());

	std::stringstream ss;
	std::string command;
	std::string reply;

	ss << "set " << m_key << ":" << m_index << " 0 0 " << m_inblock;

	command = ss.str();
	log::debug << mbstows(command) << Endl;

	if (!m_proto->sendCommand(command))
	{
		log::error << L"Unable to store cache block; unable to send command" << Endl;
		return false;
	}

	if (!m_proto->writeData(m_block, m_inblock))
	{
		log::error << L"Unable to store cache block; unable to write data" << Endl;
		return false;
	}

	if (!m_proto->readReply(reply))
	{
		log::error << L"Unable to store cache block; unable to read reply" << Endl;
		return false;
	}

	if (reply != "STORED")
	{
		log::error << L"Unable to store cache block; server unable to store data" << Endl;
		return false;
	}

	m_index++;
	return true;
}

	}
}
