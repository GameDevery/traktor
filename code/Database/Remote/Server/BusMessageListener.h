#pragma once

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Event bus message listener.
 * \ingroup Database
 */
class BusMessageListener : public IMessageListenerImpl< BusMessageListener >
{
	T_RTTI_CLASS;

public:
	BusMessageListener(Connection* connection);

private:
	Connection* m_connection;

	bool messagePutEvent(const class DbmPutEvent* message);

	bool messageGetEvent(const class DbmGetEvent* message);
};

	}
}

