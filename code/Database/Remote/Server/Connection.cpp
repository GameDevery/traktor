#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Database/Remote/IMessage.h"
#include "Database/Remote/Server/BusMessageListener.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/ConnectionMessageListener.h"
#include "Database/Remote/Server/DatabaseMessageListener.h"
#include "Database/Remote/Server/GroupMessageListener.h"
#include "Database/Remote/Server/InstanceMessageListener.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/StreamServer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Connection", Connection, Object)

Connection::Connection(
	Semaphore& connectionStringsLock,
	const std::map< std::wstring, std::wstring >& connectionStrings,
	net::StreamServer* streamServer,
	net::TcpSocket* clientSocket
)
:	m_streamServer(streamServer)
,	m_clientSocket(clientSocket)
,	m_nextHandle(1)
{
	m_transport = new net::BidirectionalObjectTransport(clientSocket);

	m_messageListeners.push_back(new BusMessageListener(this));
	m_messageListeners.push_back(new ConnectionMessageListener(this));
	m_messageListeners.push_back(new DatabaseMessageListener(connectionStringsLock, connectionStrings, m_streamServer->getListenPort(), this));
	m_messageListeners.push_back(new GroupMessageListener(this));
	m_messageListeners.push_back(new InstanceMessageListener(this));

	ThreadPool::getInstance().spawn(makeFunctor(this, &Connection::messageThread), m_thread, Thread::Above);
	T_ASSERT(m_thread);
}

void Connection::destroy()
{
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		m_thread = nullptr;
	}

	if (m_transport)
		m_transport = nullptr;

	if (m_clientSocket)
	{
		m_clientSocket->close();
		m_clientSocket = nullptr;
	}

	m_objectStore.clear();
}

bool Connection::alive() const
{
	return m_thread != nullptr;
}

void Connection::sendReply(const IMessage& message)
{
	if (!m_transport->send(&message))
	{
		log::error << L"Unable to send reply (" << type_name(&message) << L"); connection terminated." << Endl;
		destroy();
	}
}

uint32_t Connection::putObject(Object* object)
{
	uint32_t handle = m_nextHandle++;
	m_objectStore[handle] = object;
	return handle;
}

Ref< Object > Connection::getObject(uint32_t handle)
{
	std::map< uint32_t, Ref< Object > >::iterator i = m_objectStore.find(handle);
	return i != m_objectStore.end() ? i->second : nullptr;
}

void Connection::releaseObject(uint32_t handle)
{
	m_objectStore.erase(handle);
}

void Connection::setDatabase(IProviderDatabase* database)
{
	m_database = database;
}

IProviderDatabase* Connection::getDatabase() const
{
	return m_database;
}

net::StreamServer* Connection::getStreamServer() const
{
	return m_streamServer;
}

void Connection::messageThread()
{
	while (!m_thread->stopped())
	{
		Ref< IMessage > message;
		if (m_transport->recv< IMessage >(100, message) < 0)
			break;

		if (!message)
			continue;

		for (auto listener : m_messageListeners)
		{
			if (listener->notify(message))
			{
				message = nullptr;
				break;
			}
		}

		if (message)
		{
			log::error << L"Unhandled message \"" << type_name(message) << L"\"; connection terminated." << Endl;
			break;
		}
	}
	m_thread = nullptr;
}

	}
}
