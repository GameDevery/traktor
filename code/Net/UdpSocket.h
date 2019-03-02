#pragma once

#include "Core/Ref.h"
#include "Net/Socket.h"

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

class SocketAddress;
class SocketAddressIPv4;
class SocketAddressIPv6;

/*! \brief UDP datagram socket.
 * \ingroup Net
 */
class T_DLLCLASS UdpSocket : public Socket
{
	T_RTTI_CLASS;

public:
	UdpSocket();

	virtual void close() override final;

	/*! \brief Bind to local address. */
	bool bind(const SocketAddressIPv4& socketAddress);

	/*! \brief Bind to local address. */
	bool bind(const SocketAddressIPv6& socketAddress);

	/*! \brief Connect to remote host. */
	bool connect(const SocketAddressIPv4& socketAddress);

	/*! \brief Connect to remote host. */
	bool connect(const SocketAddressIPv6& socketAddress);

	/*! \brief Get local socket address. */
	Ref< SocketAddress > getLocalAddress();

	/*! \brief Send data directly to address. */
	int sendTo(const SocketAddressIPv4& socketAddress, const void* data, int length);

	/*! \breif Receive block of data. */
	int recvFrom(void* data, int length, SocketAddressIPv4* outSocketAddress);
};

	}
}

