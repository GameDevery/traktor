#pragma once

#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;
class TcpSocket;

	}

	namespace db
	{

/*! \brief Database connection.
 * \ingroup Database
 */
class RemoteConnection : public Object
{
	T_RTTI_CLASS;

public:
	RemoteConnection(net::TcpSocket* socket);

	void destroy();

	void setStreamServerAddr(const net::SocketAddressIPv4& streamServerAddr);

	const net::SocketAddressIPv4& getStreamServerAddr() const;

	template < typename ReplyMessageType >
	Ref< ReplyMessageType > sendMessage(const IMessage& message)
	{
		Ref< IMessage > reply = sendMessage(message);
		return dynamic_type_cast< ReplyMessageType* >(reply);
	}

private:
	Ref< net::TcpSocket > m_socket;
	net::SocketAddressIPv4 m_streamServerAddr;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Semaphore m_transportLock;

	Ref< IMessage > sendMessage(const IMessage& message);
};

	}
}

