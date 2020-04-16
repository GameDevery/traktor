#include <algorithm>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ConnectionManager", ConnectionManager, Object)

ConnectionManager::ConnectionManager(net::StreamServer* streamServer)
:	m_streamServer(streamServer)
,	m_listenPort(0)
,	m_serverThread(0)
{
}

bool ConnectionManager::create()
{
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(0)))
	{
		log::error << L"Failed to create remote database connection manager; unable to bind server socket." << Endl;
		return false;
	}
	if (!m_listenSocket->listen())
	{
		log::error << L"Failed to create remote database connection manager; unable to listenen." << Endl;
		return false;
	}

	m_listenPort = dynamic_type_cast< net::SocketAddressIPv4* >(m_listenSocket->getLocalAddress())->getPort();

	ThreadPool::getInstance().spawn(
		makeFunctor(this, &ConnectionManager::threadServer),
		m_serverThread
	);
	if (!m_serverThread)
		return false;

	log::info << L"Remote database connection manager @" << m_listenPort << L" created." << Endl;
	return true;
}

void ConnectionManager::destroy()
{
	if (m_serverThread)
	{
		ThreadPool::getInstance().stop(m_serverThread);
		m_serverThread = nullptr;
	}

	if (m_listenSocket)
	{
		m_listenSocket->close();
		m_listenSocket = nullptr;
	}

	for (auto connection : m_connections)
		connection->destroy();

	m_connections.clear();

	m_streamServer = nullptr;
}

void ConnectionManager::setConnectionString(const std::wstring& name, const std::wstring& connectionString)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionStringsLock);
	m_connectionStrings[name] = connectionString;
}

void ConnectionManager::removeConnectionString(const std::wstring& name)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionStringsLock);
	m_connectionStrings.erase(name);
}

uint16_t ConnectionManager::getListenPort() const
{
	return m_listenPort;
}

void ConnectionManager::threadServer()
{
	while (!m_serverThread->stopped())
	{
		if (m_listenSocket->select(true, false, false, 100) > 0)
		{
			Ref< net::TcpSocket > clientSocket = m_listenSocket->accept();
			if (!clientSocket)
				continue;

			Ref< Connection > connection = new Connection(m_connectionStringsLock, m_connectionStrings, m_streamServer, clientSocket);
			m_connections.push_back(connection);

			Ref< net::SocketAddress > remoteAddress = clientSocket->getRemoteAddress();
			if (remoteAddress)
				log::info << L"Remote database connection accepted from " << remoteAddress->getHostName() << L"." << Endl;
			else
				log::info << L"Remote database connection accepted." << Endl;
		}
		else
		{
			uint32_t count = uint32_t(m_connections.size());
			for (RefArray< Connection >::iterator i = m_connections.begin(); i != m_connections.end(); )
			{
				if (!(*i)->alive())
					i = m_connections.erase(i);
				else
					++i;
			}
			count -= uint32_t(m_connections.size());
			if (count)
				log::info << count << L" remote database connection(s) removed." << Endl;
		}
	}
}

	}
}
