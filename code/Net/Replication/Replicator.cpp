//#define T_ENABLE_MEASURE

#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Timer/Measure.h"
#include "Net/Replication/IReplicatorEventListener.h"
#include "Net/Replication/IReplicatorStateListener.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/ReplicatorTypes.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const double c_catastrophicDeltaTime = 30.0;
const double c_maxDeltaTime = 0.1;
const uint32_t c_maxDeltaTimeCount = 10;

bool lowestLatencyPred(const ReplicatorProxy* l, const ReplicatorProxy* r)
{
	return l->getLatency() + l->getLatencySpread() < r->getLatency() + r->getLatencySpread();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator", Replicator, Object)

Replicator::Replicator()
:	m_time0(0.0)
,	m_timeContinuousSync(0.0)
,	m_time(0.0)
,	m_timeVariance(0.0)
,	m_status(0)
,	m_allowPrimaryRequests(true)
,	m_origin(Transform::identity())
,	m_sendState(false)
,	m_timeSynchronization(true)
,	m_timeSynchronized(false)
,	m_exceededDeltaTimeLimit(0)
{
}

Replicator::~Replicator()
{
	destroy();
}

bool Replicator::create(INetworkTopology* topology, const Configuration& configuration)
{
	m_topology = topology;
	m_topology->setCallback(this);
	m_configuration = configuration;
	return true;
}

void Replicator::destroy()
{
	removeAllEventTypes();
	removeAllListeners();

	if (m_topology)
	{
		m_topology->setCallback(0);
		m_topology = 0;
	}
}

void Replicator::setConfiguration(const Configuration& configuration)
{
	m_configuration = configuration;
}

const Replicator::Configuration& Replicator::getConfiguration() const
{
	return m_configuration;
}

void Replicator::addEventType(const TypeInfo& eventType)
{
	m_eventTypes.push_back(&eventType);
}

void Replicator::removeAllEventTypes()
{
	m_eventTypes.resize(0);
}

IReplicatorStateListener* Replicator::addListener(IReplicatorStateListener* listener)
{
	m_listeners.push_back(listener);
	return listener;
}

void Replicator::removeListener(IReplicatorStateListener* listener)
{
	m_listeners.remove(listener);
}

void Replicator::removeAllListeners()
{
	m_listeners.clear();
}

IReplicatorEventListener* Replicator::addEventListener(const TypeInfo& eventType, IReplicatorEventListener* eventListener)
{
	m_eventListeners[&eventType].push_back(eventListener);
	return eventListener;
}

void Replicator::removeEventListener(IReplicatorEventListener* eventListener)
{
	for (SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >::iterator i = m_eventListeners.begin(); i != m_eventListeners.end(); ++i)
		i->second.remove(eventListener);
}

void Replicator::removeAllEventListeners()
{
	m_eventListeners.clear();
}

bool Replicator::update()
{
	T_MEASURE_BEGIN();

	RMessage msg;
	RMessage reply;
	net_handle_t from;

	// Need to use real-time delta; cannot use engine filtered and clamped delta.
	double dT = 0.0;
	if (m_timer.started())
	{
		dT = m_timer.getDeltaTime();
		if (dT > c_catastrophicDeltaTime)
		{
			log::error << getLogPrefix() << L"Catastrophic delta time measured (" << dT << L" second(s)), cannot sustain reliable networking." << Endl;
			return false;
		}
	}
	else
	{
		// Do nothing on first iteration, just start timer so we get a correct reading next time.
		m_timer.start();
		return true;
	}

	// Update underlying network topology layer.
	if (!m_topology->update(dT))
		return false;

	T_MEASURE_UNTIL(0.01);

	// Send ping to proxies.
	{
		msg.id = RmiPing;
		msg.time = time2net(m_time);
		msg.ping.time0 = time2net(m_time0);
		msg.ping.status = m_status;

		for (auto proxy : m_proxies)
		{
			if ((proxy->m_timeUntilTxPing -= dT) <= 0.0)
			{
				T_MEASURE_STATEMENT(m_topology->send(proxy->m_handle, &msg, RmiPing_NetSize()), 0.001);
				proxy->m_timeUntilTxPing = m_configuration.timeUntilTxPing;
			}
		}
	}

	T_MEASURE_UNTIL(0.002);

	// Send our state to proxies.
	if (m_sendState && m_stateTemplate && m_state)
	{
		msg.id = RmiState;
		msg.time = time2net(m_time);

		uint32_t stateDataSize = 0;
		T_MEASURE_STATEMENT(stateDataSize = m_stateTemplate->pack(
			m_state,
			msg.state.data,
			RmiState_MaxStateSize()
		), 0.001);

		if (stateDataSize > 0)
		{
			for (auto proxy : m_proxies)
			{
				if (proxy->m_sendState && (proxy->m_timeUntilTxState -= dT) <= 0.0)
				{
					T_MEASURE_STATEMENT(m_topology->send(proxy->m_handle, &msg, RmiState_NetSize(stateDataSize)), 0.001);

					Vector4 direction = proxy->m_origin.translation() - m_origin.translation();
					Scalar distance = direction.length();

					float t = clamp((distance - m_configuration.nearDistance) / (m_configuration.farDistance - m_configuration.nearDistance), 0.0f, 1.0f);

					proxy->m_distance = distance;
					proxy->m_timeUntilTxState = lerp(m_configuration.timeUntilTxStateNear, m_configuration.timeUntilTxStateFar, t);
				}
			}
		}
	}

	T_MEASURE_UNTIL(0.002);

	double timeOffset = 0.0;
	bool timeOffsetReceived = false;

	// Receive messages.
	for (;;)
	{
		from = 0;

		// Poll message from topology.
		int32_t nrecv = 0;
		T_MEASURE_STATEMENT(nrecv = m_topology->recv(&msg, sizeof(msg), from), 0.001);
		if (nrecv <= 0)
			break;

		// Find proxy from which we received a message.
		Ref< ReplicatorProxy > fromProxy;
		for (auto proxy : m_proxies)
		{
			if (proxy->m_handle == from)
			{
				fromProxy = proxy;
				break;
			}
		}

		if (!fromProxy)
		{
			log::error << getLogPrefix() << L"Received message (" << int32_t(msg.id) << L") from unknown proxy " << from << L"; message ignored." << Endl;
			continue;
		}

		if (fromProxy->isPrimary() && fromProxy->isLatencyReliable())
		{
			double latency = fromProxy->getReverseLatency();
			double ghostOffset = net2time(msg.time) + latency - m_timeContinuousSync;

			if (!timeOffsetReceived)
				timeOffset = ghostOffset;
			else
			{
				if (ghostOffset > 0.0f && timeOffset > 0.0f)
					timeOffset = std::max(timeOffset, ghostOffset);
				else if (ghostOffset < 0.0f && timeOffset < 0.0f)
					timeOffset = std::min(timeOffset, ghostOffset);
			}

			timeOffsetReceived = true;
		}

		if (msg.id == RmiPing)
		{
			if (msg.ping.status != fromProxy->m_status)
			{
				fromProxy->m_status = msg.ping.status;
				for (auto listener : m_listeners)
				{
					listener->notify(
						this,
						float(m_time),
						IReplicatorStateListener::ReStatus,
						fromProxy,
						0
					);
				}
			}

			reply.id = RmiPong;
			reply.time = time2net(m_time);
			reply.pong.time0 = msg.ping.time0;
			reply.pong.rtime0 = time2net(m_time0);
			reply.pong.latency = time2net(fromProxy->getLatency());
			reply.pong.latencySpread = time2net(fromProxy->getLatencySpread());

			T_MEASURE_STATEMENT(m_topology->send(fromProxy->m_handle, &reply, RmiPong_NetSize()), 0.001);
		}
		else if (msg.id == RmiPong)
		{
			double pingTime = min(m_time0, net2time(msg.pong.time0));
			double roundTrip = m_time0 - pingTime;
			double latencyReverse = net2time(msg.pong.latency);
			double latencyReverseSpread = net2time(msg.pong.latencySpread);

			T_MEASURE_STATEMENT(fromProxy->updateLatency(m_time0, net2time(msg.pong.rtime0), roundTrip, latencyReverse, latencyReverseSpread), 0.001);
		}
		else if (msg.id == RmiState)
		{
			bool received;
			T_MEASURE_STATEMENT(received = fromProxy->receivedState(m_time, net2time(msg.time), msg.state.data, RmiState_StateSize(nrecv)), 0.001);
			if (received)
				fromProxy->m_issueStateListeners = true;
		}
		else if (msg.id == RmiEvent0 || msg.id == RmiEvent1)
		{
			// Unwrap event object.
			MemoryStream ms(msg.event.data, RmiEvent_EventSize(nrecv), true, false);
			Ref< ISerializable > eventObject;
			T_MEASURE_STATEMENT(eventObject = CompactSerializer(&ms, &m_eventTypes[0], uint32_t(m_eventTypes.size())).readObject< ISerializable >(), 0.001);

			if (eventObject && ms.tell() == RmiEvent_EventSize(nrecv))
			{
				if (fromProxy->receivedRxEvent(msg.time, msg.event.sequence, eventObject, msg.id == RmiEvent1))
				{
					// Send back event acknowledge.
					reply.id = (msg.id == RmiEvent0) ? RmiEvent0Ack : RmiEvent1Ack;
					reply.time = time2net(m_time);
					reply.eventAck.sequence = msg.event.sequence;
					T_MEASURE_STATEMENT(m_topology->send(fromProxy->m_handle, &reply, RmiEventAck_NetSize()), 0.001);
				}
				else
					log::error << getLogPrefix() << L"Unable to enqueue event object." << Endl;
			}
			else
				log::error << getLogPrefix() << L"Unable to deserialize event object, possibly corrupt data received." << Endl;
		}
		else if (msg.id == RmiEvent0Ack || msg.id == RmiEvent1Ack)
		{
			// Received an event acknowledge; discard event from queue.
			T_MEASURE_STATEMENT(fromProxy->receivedTxEventAcknowledge(fromProxy, msg.eventAck.sequence, msg.id == RmiEvent1Ack), 0.001);
		}
	}

	T_MEASURE_UNTIL(0.004);

	/*
	// \note Disabled for now as it's not used by GU.
	// Invoke listeners for each new state.
	for (auto proxy : m_proxies)
	{
		if proxy->m_issueStateListeners)
		{
			for (auto listener : m_listeners)
			{
				T_MEASURE_STATEMENT(listener->notify(
					this,
					0.0f,
					IListener::ReState,
					proxy,
					proxy->m_state0
				), 0.001);
			}
			proxy->m_issueStateListeners = false;
		}
	}
	*/

	// Update proxy queues.
	for (auto proxy : m_proxies)
	{
		T_MEASURE_STATEMENT(proxy->updateTxEventQueue(), 0.001);
		T_MEASURE_STATEMENT(proxy->dispatchRxEvents(m_eventListeners), 0.001);
	}

	T_MEASURE_UNTIL(0.001);

	if (m_timeSynchronization && timeOffsetReceived)
	{
		if (abs(timeOffset) >= 1.0)
		{
			if (m_timeSynchronized)
			{
#if defined(_DEBUG)
				log::warning << getLogPrefix() << L"Time synchronization lost (1)." << Endl;
#endif
				m_timeErrors.clear();
				m_timeSynchronized = false;
			}
			timeOffset *= 0.8;
		}
		else if (abs(timeOffset) > 0.06)
		{
			if (abs(timeOffset) > 0.2)
			{
				if (m_timeSynchronized)
				{
#if defined(_DEBUG)
					log::warning << getLogPrefix() << L"Time synchronization lost (2)." << Endl;
#endif
					m_timeErrors.clear();
					m_timeSynchronized = false;
				}
			}
			timeOffset *= 0.4;
		}
		else
		{
			double k = abs(timeOffset) / 0.06;
			timeOffset *= 0.4 * k;
		}

		m_timeContinuousSync += timeOffset;

		// Update time variance; this should become fairly stable after a couple of estimates.
		m_timeErrors.push_back(abs(timeOffset));
		{
			double k = 0.0;
			for (uint32_t i = 0; i < m_timeErrors.size(); ++i)
				k += m_timeErrors[i];
			k /= double(m_timeErrors.size());
			m_timeVariance = std::sqrt((k * k) / 6.0);
		}

		// Have we reached acceptable variance?
		if (
			m_timeErrors.full() &&
			m_timeVariance <= m_configuration.timeVarianceThreshold &&
			abs(timeOffset) <= 0.02
		)
		{
			if (!m_timeSynchronized)
			{
#if defined(_DEBUG)
				log::info << getLogPrefix() << L"Time synchronized (" << (m_timeContinuousSync - m_time) * 1000.0 << L" ms)" << Endl;
#endif

				for (auto proxy : m_proxies)
				{
					proxy->m_stateTimeN2 += (m_timeContinuousSync - m_time);
					proxy->m_stateTimeN1 += (m_timeContinuousSync - m_time);
					proxy->m_stateTime0 += (m_timeContinuousSync - m_time);
				}

				m_time = m_timeContinuousSync;
				m_timeSynchronized = true;
			}
		}
	}

	T_MEASURE_UNTIL(0.001);

	// Migrate primary peer token to most suitable.
	if (isPrimary())
	{
		// Time is considered to be sync if I'm primary.
		m_timeSynchronized = true;

		// Count number of exceeded dT; should migrate primary if we're running poorly.
		if (dT > c_maxDeltaTime)
			m_exceededDeltaTimeLimit++;
		else
			m_exceededDeltaTimeLimit = 0;

		m_proxies.sort(lowestLatencyPred);
		if ((m_status & 0x80) == 0x00 || m_exceededDeltaTimeLimit > c_maxDeltaTimeCount)
		{
			// Not "in session"; migrate primary if anyone else is.
			for (auto proxy : m_proxies)
			{
				if ((proxy->getStatus() & 0x80) == 0x80)
				{
					if (proxy->setPrimary())
					{
#if defined(_DEBUG)
						log::info << getLogPrefix() << L"Migrated primary token to peer " << proxy->getHandle() << L"." << Endl;
#endif
						break;
					}
#if defined(_DEBUG)
					else
						log::info << getLogPrefix() << L"Unable migrate primary token to peer " << proxy->getHandle() << L"." << Endl;
#endif
				}
			}
		}
	}

	T_MEASURE_UNTIL(0.001);

	m_time += dT;
	m_timeContinuousSync += dT;
	m_time0 += dT;

	return true;
}

void Replicator::flush()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	for (;;)
	{
		// Check if any proxy has pending, unacknowledged events pending.
		bool pendingEvents = false;
		for (auto proxy : m_proxies)
		{
			if (proxy->updateTxEventQueue())
				pendingEvents = true;
		}

		// If no pending events then all queues are empty thus we're flushed.
		if (!pendingEvents)
			break;

		// Update system.
		update();

		// Yield a bit of processor.
		currentThread->sleep(30);
	}
}

const std::wstring& Replicator::getName() const
{
	return m_name;
}

void Replicator::setStatus(uint8_t status)
{
	if (m_status != status)
	{
		// If status has changed we need to ping our
		// fellow peers as soon as possible to let them
		// know about our new status.
		for (auto proxy : m_proxies)
			proxy->m_timeUntilTxPing = 0.0;

		m_status = status;
	}
}

uint8_t Replicator::getStatus() const
{
	return m_status;
}

bool Replicator::isPrimary() const
{
	return m_topology->getPrimaryHandle() == m_topology->getLocalHandle();
}

void Replicator::setOrigin(const Transform& origin)
{
	m_origin = origin;
}

void Replicator::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;
}

void Replicator::setState(const State* state)
{
	// If state represent a critical change we need to send
	// the state to fellow peers as soon as possible.
	if (state != 0 && m_stateTemplate->critical(m_state, state))
	{
		for (auto proxy : m_proxies)
		{
			if (proxy->m_distance < m_configuration.furthestDistance)
				proxy->m_timeUntilTxState = 0.0;
		}
	}
	m_state = state;
}

const State* Replicator::getState() const
{
	return m_state;
}

void Replicator::setSendState(bool sendState)
{
	m_sendState = sendState;
}

uint32_t Replicator::getProxyCount() const
{
	return uint32_t(m_proxies.size());
}

ReplicatorProxy* Replicator::getProxy(uint32_t index) const
{
	return m_proxies[index];
}

bool Replicator::broadcastEvent(const ISerializable* eventObject, bool inOrder)
{
	for (auto proxy : m_proxies)
		proxy->sendEvent(eventObject, inOrder);
	return true;
}

ReplicatorProxy* Replicator::getPrimaryProxy() const
{
	for (auto proxy : m_proxies)
	{
		if (proxy->isPrimary())
			return proxy;
	}
	return nullptr;
}

void Replicator::resetAllLatencies()
{
	for (auto proxy : m_proxies)
		proxy->resetLatencies();
}

double Replicator::getAverageLatency() const
{
	double latency = 0.0;
	if (!m_proxies.empty())
	{
		int32_t reliableCount = 0;
		for (auto proxy : m_proxies)
		{
			if (proxy->isLatencyReliable())
			{
				latency += proxy->getLatency();
				reliableCount++;
			}
		}
		if (reliableCount > 0)
			latency /= double(reliableCount);
	}
	return latency;
}

double Replicator::getAverageReverseLatency() const
{
	double latency = 0.0;
	if (!m_proxies.empty())
	{
		int32_t reliableCount = 0;
		for (auto proxy : m_proxies)
		{
			if (proxy->isLatencyReliable())
			{
				latency += proxy->getReverseLatency();
				reliableCount++;
			}
		}
		if (reliableCount > 0)
			latency /= double(m_proxies.size());
	}
	return latency;
}

double Replicator::getBestLatency() const
{
	double latency = 0.0;
	if (!m_proxies.empty())
	{
		int32_t reliableCount = 0;
		latency = std::numeric_limits< double >::max();
		for (auto proxy : m_proxies)
		{
			if (proxy->isLatencyReliable())
			{
				latency = std::min(latency, proxy->getLatency());
				reliableCount++;
			}
		}
		if (reliableCount <= 0)
			latency = 0.0;
	}
	return latency;
}

double Replicator::getBestReverseLatency() const
{
	double latency = 0.0;
	if (!m_proxies.empty())
	{
		int32_t reliableCount = 0;
		latency = std::numeric_limits< double >::max();
		for (auto proxy : m_proxies)
		{
			if (proxy->isLatencyReliable())
			{
				latency = std::min(latency, proxy->getReverseLatency());
				reliableCount++;
			}
		}
		if (reliableCount <= 0)
			latency = 0.0;
	}
	return latency;
}

double Replicator::getWorstLatency() const
{
	double latency = 0.0;
	for (auto proxy : m_proxies)
	{
		if (proxy->isLatencyReliable())
			latency = std::max(latency, proxy->getLatency());
	}
	return latency;
}

double Replicator::getWorstReverseLatency() const
{
	double latency = 0.0;
	for (auto proxy : m_proxies)
	{
		if (proxy->isLatencyReliable())
			latency = std::max(latency, proxy->getReverseLatency());
	}
	return latency;
}

bool Replicator::sendEventToPrimary(const ISerializable* eventObject, bool inOrder)
{
	T_FATAL_ASSERT (eventObject);

	if (!isPrimary())
	{
		// Find primary proxy and send event to it.
		ReplicatorProxy* primaryProxy = getPrimaryProxy();
		if (primaryProxy)
		{
			primaryProxy->sendEvent(eventObject, inOrder);
			return true;
		}
		else
		{
			log::error << getLogPrefix() << L"Unable to send event " << type_name(eventObject) << L" to primary; no primary found." << Endl;
			return false;
		}
	}
	else
	{
		bool processed = false;
		uint32_t count = 0;

		// We are primary peer; dispatch event directly.
		SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >::const_iterator it = m_eventListeners.find(&type_of(eventObject));
		if (it != m_eventListeners.end())
		{
			T_ANONYMOUS_VAR(Ref< const ISerializable >)(eventObject);
			for (auto eventListener : it->second)
			{
				processed |= eventListener->notify(
					this,
					float(m_time),
					0,
					eventObject
				);
				++count;
			}
		}

		if (!processed && count > 0)
			log::warning << getLogPrefix() << L"Event " << type_name(eventObject) << L" to local not processed by " << count << L" listener(s); event discarded." << Endl;

		return true;
	}
}

void Replicator::setTimeSynchronization(bool timeSynchronization)
{
	m_timeSynchronization = timeSynchronization;
}

double Replicator::getTime() const
{
	return m_time;
}

double Replicator::getTimeVariance() const
{
	return m_timeVariance;
}

bool Replicator::isTimeSynchronized() const
{
	return m_timeSynchronized;
}

std::wstring Replicator::getLogPrefix() const
{
	return L"Replicator: [" + toString(m_topology->getLocalHandle()) + L"] ";
}

bool Replicator::nodeConnected(INetworkTopology* topology, net_handle_t node)
{
	std::wstring name;
	Ref< Object > user;

	for (int32_t i = 0; i < topology->getNodeCount(); ++i)
	{
		if (topology->getNodeHandle(i) == node)
		{
			name = topology->getNodeName(i);
			user = topology->getNodeUser(i);
			break;
		}
	}

	if (node != m_topology->getLocalHandle())
	{
		Ref< ReplicatorProxy > proxy = new ReplicatorProxy(this, node, name, user);
		m_proxies.push_back(proxy);

		log::info << getLogPrefix() << L"Proxy for node " << node << L" (" << name << L") created." << Endl;

		for (auto listener : m_listeners)
		{
			listener->notify(
				this,
				float(m_time),
				IReplicatorStateListener::ReConnected,
				proxy,
				0
			);
		}
	}
	else
	{
		m_name = name;
	}

	return true;
}

bool Replicator::nodeDisconnected(INetworkTopology* topology, net_handle_t node)
{
	for (auto proxy : m_proxies)
	{
		if (proxy->m_handle == node)
		{
			log::info << getLogPrefix() << L"Proxy for node " << node << L" (" << proxy->getName() << L") destroyed." << Endl;

			for (auto listener : m_listeners)
			{
				listener->notify(
					this,
					float(m_time),
					IReplicatorStateListener::ReDisconnected,
					proxy,
					0
				);
			}

			proxy->disconnect();
			m_proxies.remove(proxy);
			break;
		}
	}
	return true;
}

	}
}
