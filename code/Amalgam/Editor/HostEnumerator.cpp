#include "Amalgam/Editor/HostEnumerator.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.HostEnumerator", HostEnumerator, Object)

HostEnumerator::HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager)
:	m_discoveryManager(discoveryManager)
{
	std::vector< std::wstring > hosts = settings->getProperty< PropertyStringArray >(L"Amalgam.RemoteHosts");
	for (std::vector< std::wstring >::const_iterator i = hosts.begin(); i != hosts.end(); ++i)
	{
		Host h;
		h.host = *i;
		h.description = *i;
		h.local = false;
		m_manual.push_back(h);
	}
}

int32_t HostEnumerator::count() const
{
	return int32_t(m_hosts.size());
}

bool HostEnumerator::getHost(int32_t index, std::wstring& outHost) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		outHost = m_hosts[index].host;
		return true;
	}
	else
		return false;
}

bool HostEnumerator::getDescription(int32_t index, std::wstring& outDescription) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		outDescription = m_hosts[index].description;
		return true;
	}
	else
		return false;
}

bool HostEnumerator::supportPlatform(int32_t index, const std::wstring& platform) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		const std::vector< std::wstring >& platforms = m_hosts[index].platforms;
		if (!platforms.empty())
			return std::find(platforms.begin(), platforms.end(), platform) != platforms.end();
		else
			return true;
	}
	else
		return false;
}

bool HostEnumerator::isLocal(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
		return m_hosts[index].local;
	else
		return false;
}

void HostEnumerator::update()
{
	RefArray< net::NetworkService > services;
	if (!m_discoveryManager->findServices< net::NetworkService >(services, 100))
		return;

	net::SocketAddressIPv4::Interface itf;
	net::SocketAddressIPv4::getBestInterface(itf);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		m_hosts.clear();
		m_hosts.insert(m_hosts.end(), m_manual.begin(), m_manual.end());

		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"RemoteTools/Server")
				continue;

			const PropertyGroup* properties = (*i)->getProperties();
			if (!properties)
				continue;

			Host h;
			h.host = properties->getProperty< PropertyString >(L"Host");
			h.description = properties->getProperty< PropertyString >(L"Description");
			h.platforms = properties->getProperty< PropertyStringArray >(L"Platforms");
			h.local = bool(itf.addr != 0 && itf.addr->getHostName() == h.host);
			m_hosts.push_back(h);
		}

		std::sort(m_hosts.begin(), m_hosts.end());
	}
}

HostEnumerator::Host::Host()
:	local(false)
{
}

bool HostEnumerator::Host::operator < (const Host& h) const
{
	return description < h.description;
}

	}
}
