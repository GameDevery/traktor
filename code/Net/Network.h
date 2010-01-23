#ifndef traktor_net_Network_H
#define traktor_net_Network_H

#include "Core/Config.h"

#if defined(__GNUC__) || defined(_PS3) || defined(__APPLE__)
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <unistd.h>

#	if defined(_PS3)
#		include <netex/net.h>
#		include <netex/ifctl.h>
#		include <netex/libnetctl.h>
#		include <cell/sysmodule.h>
#		include <sys/timer.h>
#		include <sysutil/sysutil_common.h>
#	endif

#	define SOCKET int
#	define INVALID_SOCKET -1
#	define CLOSE_SOCKET(s) ::close(s)

#	if !defined(SOMAXCONN)
#		define SOMAXCONN 4
#	endif
#elif defined(_WIN32)
#	if defined(_XBOX)
#		include <xtl.h>
#	else
#		define _WIN32_LEAN_AND_MEAN
#		include <winsock2.h>
#		include <ws2tcpip.h>
#		include <windows.h>
#	endif

typedef int socklen_t;
#	define CLOSE_SOCKET(s) closesocket(s)

#	if defined(max)
#		undef max
#	endif
#	if defined(min)
#		undef min
#endif

#endif

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS Network
{
public:
	static bool initialize();

	static void finalize();

private:
	static bool ms_initialized;
};

	}
}

#endif	// traktor_net_Network_H
