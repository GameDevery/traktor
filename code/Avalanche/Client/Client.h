/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Avalanche/Dictionary.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Net/SocketAddressIPv4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class SocketStream;

}

namespace traktor::avalanche
{

class T_DLLCLASS Client : public Object
{
	T_RTTI_CLASS;

public:
	explicit Client(const net::SocketAddressIPv4& serverAddress);

	void destroy();

	bool ping();

	bool have(const Key& key);

	bool touch(const AlignedVector< Key >& keys);

	bool evict(const AlignedVector< Key >& keys);

	Ref< IStream > get(const Key& key);

	Ref< IStream > put(const Key& key);

	bool stats(Dictionary::Stats& outStats);

	bool getKeys(AlignedVector< Key >& outKeys);

	const net::SocketAddressIPv4& getServerAddress() const { return m_serverAddress; }

private:
	friend class ClientGetStream;
	friend class ClientPutStream;

	net::SocketAddressIPv4 m_serverAddress;
	RefArray< net::SocketStream > m_streams;
	Semaphore m_lock;

	Ref< net::SocketStream > establish(uint8_t command);
};

}
