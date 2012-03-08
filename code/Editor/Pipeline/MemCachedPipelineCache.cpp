#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/MemCachedProto.h"
#include "Editor/Pipeline/MemCachedGetStream.h"
#include "Editor/Pipeline/MemCachedPutStream.h"
#include "Net/Network.h"
#include "Net/TcpSocket.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

std::string generateKey(const Guid& guid, uint32_t hash, int32_t version)
{
	std::stringstream ss;
	ss << wstombs(guid.format()) << ":" << hash << ":" << version;
	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPipelineCache", MemCachedPipelineCache, IPipelineCache)

MemCachedPipelineCache::MemCachedPipelineCache()
:	m_accessRead(true)
,	m_accessWrite(true)
{
	net::Network::initialize();
}

MemCachedPipelineCache::~MemCachedPipelineCache()
{
	destroy();
	net::Network::finalize();
}

bool MemCachedPipelineCache::create(const PropertyGroup* settings)
{
	std::wstring host = settings->getProperty< PropertyString >(L"Pipeline.MemCached.Host");
	int32_t port = settings->getProperty< PropertyInteger >(L"Pipeline.MemCached.Port", 11211);

	m_socket = new net::TcpSocket();
	if (!m_socket->connect(net::SocketAddressIPv4(host, port)))
		return false;

	m_accessRead = settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", true);
	m_accessWrite = settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", true);
	m_proto = new MemCachedProto(m_socket);

	return true;
}

void MemCachedPipelineCache::destroy()
{
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}

	m_proto = 0;
}

Ref< IStream > MemCachedPipelineCache::get(const Guid& guid, uint32_t hash, int32_t version)
{
	if (m_accessRead)
	{
		Ref< MemCachedGetStream > stream = new MemCachedGetStream(m_proto, generateKey(guid, hash, version));
		
		// Request end block; do not try to open non-finished cache streams.
		if (!stream->requestEndBlock())
			return 0;
			
		// Request first block of data.
		if (!stream->requestNextBlock())
			return 0;
			
		return stream;
	}
	else
		return 0;
}

Ref< IStream > MemCachedPipelineCache::put(const Guid& guid, uint32_t hash, int32_t version)
{
	if (m_accessWrite)
		return new MemCachedPutStream(m_proto, generateKey(guid, hash, version));
	else
		return 0;
}

	}
}
