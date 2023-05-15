/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Jungle/OnlinePeer2PeerProvider.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"

namespace traktor::jungle
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.OnlinePeer2PeerProvider", OnlinePeer2PeerProvider, IPeer2PeerProvider)

OnlinePeer2PeerProvider::OnlinePeer2PeerProvider(online::ISessionManager* sessionManager, online::ILobby* lobby, bool asyncTx, bool asyncRx)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_localHandle(sessionManager->getUser()->getGlobalId())
,	m_primaryHandle(0)
,	m_whenUpdate(0.0)
,	m_asyncTx(asyncTx)
,	m_asyncRx(asyncRx)
,	m_thread(nullptr)
,	m_rxQueuePending(0)
{
	Ref< online::IUser > fromUser;
	uint8_t data[1600];

	// Purge pending data.
	while(m_sessionManager->receiveP2PData(data, sizeof(data), fromUser) > 0)
		;

	// Create transmission thread.
	if (asyncTx || asyncRx)
	{
		if (!ThreadPool::getInstance().spawn([this](){ transmissionThread(); }, m_thread))
		{
			m_thread = nullptr;
			m_asyncTx =
			m_asyncRx = false;
		}
	}

	m_timer.reset();
}

OnlinePeer2PeerProvider::~OnlinePeer2PeerProvider()
{
	// Disable P2P with all connected users.
	for (auto& user : m_users)
		user.user->setP2PEnable(false);

	// Terminate transmission thread.
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		m_thread = nullptr;
	}
}

bool OnlinePeer2PeerProvider::update()
{
	if (m_timer.getElapsedTime() >= m_whenUpdate)
	{
		RefArray< online::IUser > users;
		m_lobby->getParticipants(users);

		// Add new users which have entered the lobby.
		for (auto user : users)
		{
			auto it = std::find_if(m_users.begin(), m_users.end(), [&](const P2PUser& u) {
				return u.user->getGlobalId() == user->getGlobalId();
			});
			if (it == m_users.end())
			{
				user->setP2PEnable(true);

				P2PUser p2pu;
				p2pu.user = user;
				p2pu.timeout = 0;
				m_users.push_back(p2pu);

				log::info << L"[Online P2P] Peer " << user->getGlobalId() << L" added." << Endl;
			}
		}

		// Increment timeout counter for users which have left.
		for (AlignedVector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
		{
			if (std::find(users.begin(), users.end(), i->user) != users.end())
			{
				if (i->timeout > 0)
				{
					log::info << L"[Online P2P] Resurrected peer " << i->user->getGlobalId() << L"." << Endl;
					i->timeout = 0;
				}
			}
			else
				i->timeout++;
		}

		// Remove users which have a timeout greater than limit.
		for (size_t i = 0; i < m_users.size(); )
		{
			if (m_users[i].timeout >= 4 || !m_users[i].user)
			{
				if (m_users[i].user)
				{
					m_users[i].user->setP2PEnable(false);
					log::info << L"[Online P2P] Peer " << m_users[i].user->getGlobalId() << L" removed." << Endl;
				}
				m_users.erase(m_users.begin() + i);
			}
			else
				++i;
		}

		// Cache primary handle.
		m_primaryHandle = net_handle_t(m_lobby->getOwner()->getGlobalId());
		m_whenUpdate = m_timer.getElapsedTime() + 0.2;
	}
	return true;
}

net_handle_t OnlinePeer2PeerProvider::getLocalHandle() const
{
	return m_localHandle;
}

int32_t OnlinePeer2PeerProvider::getPeerCount() const
{
	return 1 + int32_t(m_users.size());
}

net_handle_t OnlinePeer2PeerProvider::getPeerHandle(int32_t index) const
{
	if (index <= 0)
		return getLocalHandle();
	else
		return net_handle_t(m_users[index - 1].user->getGlobalId());
}

std::wstring OnlinePeer2PeerProvider::getPeerName(int32_t index) const
{
	std::wstring name;
	if (index <= 0)
		m_sessionManager->getUser()->getName(name);
	else
		m_users[index - 1].user->getName(name);
	return name;
}

Object* OnlinePeer2PeerProvider::getPeerUser(int32_t index) const
{
	if (index <= 0)
		return m_sessionManager->getUser();
	else
		return m_users[index - 1].user;
}

bool OnlinePeer2PeerProvider::setPrimaryPeerHandle(net_handle_t node)
{
	for (AlignedVector< P2PUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if (i->user->getGlobalId() == node)
		{
#if defined(_DEBUG)
			log::info << L"[Online P2P] Migrating primary token to peer " << node << L"..." << Endl;
#endif
			if (m_lobby->setOwner(i->user))
			{
				m_whenUpdate = 0.0f;
				return true;
			}
		}
	}
#if defined(_DEBUG)
	log::error << L"[Online P2P] Failed to migrate primary token to peer " << node << L"." << Endl;
#endif
	return false;
}

net_handle_t OnlinePeer2PeerProvider::getPrimaryPeerHandle() const
{
	return m_primaryHandle;
}

bool OnlinePeer2PeerProvider::send(net_handle_t node, const void* data, int32_t size)
{
	for (AlignedVector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if (i->user->getGlobalId() == node)
		{
			if (m_asyncTx)
			{
				T_ASSERT(m_thread);
				m_txQueueLock.wait();

				RxTxData& tx = m_txQueue.push_back();

				tx.user = i->user;
				tx.size = size;
				std::memcpy(tx.data, data, size);

				m_txQueueSignal.set();
				m_txQueueLock.release();

				return true;
			}
			else
			{
				return i->user->sendP2PData(data, size, false);
			}
		}
	}
	log::error << L"[Online P2P] Unable to send to " << node << L"; no such node." << Endl;
	return false;
}

int32_t OnlinePeer2PeerProvider::recv(void* data, int32_t size, net_handle_t& outNode)
{
	int32_t nrecv = 0;

	outNode = 0;

	if (m_asyncRx)
	{
		T_ASSERT(m_thread);

		if (m_rxQueuePending <= 0)
			return 0;

		m_rxQueueLock.wait();
		if (!m_rxQueue.empty())
		{
			RxTxData& rx = m_rxQueue.front();

			auto it = std::find_if(m_users.begin(), m_users.end(), [&](const P2PUser& u) {
				return u.user->getGlobalId() == rx.user->getGlobalId();
			});
			if (it != m_users.end())
			{
				nrecv = std::min< int32_t >(size, rx.size);
				std::memcpy(data, rx.data, nrecv);
				outNode = net_handle_t(rx.user->getGlobalId());
			}
			else
				log::warning << L"[Online P2P] Received data from unknown user " << rx.user->getGlobalId() << L" (1)." << Endl;

			m_rxQueue.pop_front();
			--m_rxQueuePending;
		}
		m_rxQueueLock.release();
	}
	else
	{
		if (m_sessionManager->haveP2PData())
		{
			Ref< online::IUser > fromUser;
			nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
			if (nrecv <= 0)
				return nrecv;
			if (!fromUser)
				return 0;

			auto it = std::find_if(m_users.begin(), m_users.end(), [&](const P2PUser& u) {
				return u.user->getGlobalId() == fromUser->getGlobalId();
			});
			if (it != m_users.end())
				outNode = net_handle_t(fromUser->getGlobalId());
			else
			{
				log::warning << L"[Online P2P] Received data from unknown user " << fromUser->getGlobalId() << L" (2)." << Endl;
				nrecv = 0;
			}
		}
	}

	return nrecv;
}

void OnlinePeer2PeerProvider::transmissionThread()
{
	T_ASSERT(m_asyncTx || m_asyncRx);
	RxTxData rxtx;

	while (!m_thread->stopped())
	{
		if (m_asyncRx)
		{
			if (m_sessionManager->haveP2PData())
			{
				rxtx.size = m_sessionManager->receiveP2PData(rxtx.data, sizeof(rxtx.data), rxtx.user);
				if (rxtx.size > 0 && rxtx.user)
				{
					m_rxQueueLock.wait();
					m_rxQueue.push_back(rxtx);
					++m_rxQueuePending;
					m_rxQueueLock.release();
				}
				continue;
			}
		}

		if (m_asyncTx)
		{
			if (m_txQueueSignal.wait(m_asyncRx ? 4 : 100))
			{
				m_txQueueLock.wait();
				rxtx = m_txQueue.front();
				m_txQueue.pop_front();
				if (m_txQueue.empty())
					m_txQueueSignal.reset();
				m_txQueueLock.release();

				rxtx.user->sendP2PData(rxtx.data, rxtx.size, false);
			}
		}
		else
		{
			T_ASSERT(m_asyncRx);
			m_thread->sleep(4);
		}
	}
}

}
