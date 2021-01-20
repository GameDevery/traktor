#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_REMOTE_SERVER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace net
    {

class DiscoveryManager;
class TcpSocket;

    }

    namespace remote
    {

class T_DLLCLASS Server : public Object
{
    T_RTTI_CLASS;

public:
	Server();

    bool create(const std::wstring& scratchPath, const std::wstring& keyword, int32_t listenPort, bool verbose);

    void destroy();

    bool update();

    int32_t getListenPort() const { return m_listenPort; }

    const std::wstring& getScratchPath() const { return m_scratchPath; }

private:
    Ref< net::TcpSocket > m_serverSocket;
    Ref< net::DiscoveryManager > m_discoveryManager;
	int32_t m_listenPort;
    std::wstring m_hostName;
    std::wstring m_scratchPath;
    std::wstring m_keyword;
    std::map< std::wstring, uint32_t > m_fileHashes;
    bool m_verbose;

    uint8_t handleDeploy(net::TcpSocket* clientSocket);

    uint8_t handleLaunchProcess(net::TcpSocket* clientSocket);

    uint8_t handleFetch(net::TcpSocket* clientSocket);

    void processClient(net::TcpSocket* clientSocket);
};

    }
}