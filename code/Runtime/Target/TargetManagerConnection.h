#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace runtime
	{

/*! Application target manager connection.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetManagerConnection : public Object
{
	T_RTTI_CLASS;

public:
	bool connect(const std::wstring& host, uint16_t port, const Guid& id);

	bool connected() const;

	bool update();

	net::BidirectionalObjectTransport* getTransport() const { return m_transport; }

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
};

	}
}

