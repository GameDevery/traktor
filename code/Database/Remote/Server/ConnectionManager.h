/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace net
	{

class TcpSocket;
class StreamServer;

	}

	namespace db
	{

class Connection;

/*! Client connection manager.
 * \ingroup Database
 */
class T_DLLCLASS ConnectionManager : public Object
{
	T_RTTI_CLASS;

public:
	ConnectionManager(net::StreamServer* streamServer);

	bool create();

	void destroy();

	void setConnectionString(const std::wstring& name, const std::wstring& connectionString);

	void removeConnectionString(const std::wstring& name);

	uint16_t getListenPort() const;

private:
	Ref< net::StreamServer > m_streamServer;
	uint16_t m_listenPort;
	Ref< net::TcpSocket > m_listenSocket;
	Thread* m_serverThread;
	RefArray< Connection > m_connections;
	Semaphore m_connectionStringsLock;
	SmallMap< std::wstring, std::wstring > m_connectionStrings;

	void threadServer();
};

	}
}

