#ifndef traktor_net_ReliableTransportPeers_H
#define traktor_net_ReliableTransportPeers_H

#include <list>
#include <map>
#include "Core/Timer//Timer.h"
#include "Net/Replication/IReplicatorPeers.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS ReliableTransportPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	ReliableTransportPeers(IReplicatorPeers* peers);

	virtual ~ReliableTransportPeers();

	virtual void destroy();

	virtual void update();

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

	virtual bool isPrimary() const;

private:
	enum EnvelopeType
	{
		EtUnreliable = 0x01,
		EtReliable = 0x02,
		EtAck = 0x03
	};

#pragma pack(1)
	struct Envelope
	{
		uint8_t type;
		uint8_t sequence;
		uint8_t payload[1200];
	};
#pragma pack()

	struct ControlEnvelope
	{
		double time0;
		double time;
		uint32_t size;
		Envelope envelope;
	};

	struct Control
	{
		uint8_t sequence0;
		uint8_t sequence1;
		std::list< ControlEnvelope > sent;
		uint8_t last0_0;
		uint8_t last0_1;
		uint8_t last1_0;
		uint8_t last1_1;
		bool alive;

		Control()
		:	sequence0(0)
		,	sequence1(0)
		,	alive(false)
		,	last0_0(0xff)
		,	last0_1(0xff)
		,	last1_0(0xff)
		,	last1_1(0xff)
		{
		}
	};

	Ref< IReplicatorPeers > m_peers;
	Timer m_timer;
	std::map< handle_t, Control > m_control;
};

	}
}

#endif	// traktor_net_ReliableTransportPeers_H
