#pragma once

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Instance message listener.
 * \ingroup Database
 */
class InstanceMessageListener : public IMessageListenerImpl< InstanceMessageListener >
{
	T_RTTI_CLASS;

public:
	InstanceMessageListener(Connection* connection);

private:
	Connection* m_connection;

	bool messageGetInstancePrimaryType(const class DbmGetInstancePrimaryType* message);

	bool messageOpenTransaction(const class DbmOpenTransaction* message);

	bool messageCommitTransaction(const class DbmCommitTransaction* message);

	bool messageCloseTransaction(const class DbmCloseTransaction* message);

	bool messageGetInstanceName(const class DbmGetInstanceName* message);

	bool messageSetInstanceName(const class DbmSetInstanceName* message);

	bool messageGetInstanceGuid(const class DbmGetInstanceGuid* message);

	bool messageSetInstanceGuid(const class DbmSetInstanceGuid* message);

	bool messageRemoveInstance(const class DbmRemoveInstance* message);

	bool messageReadObject(const class DbmReadObject* message);

	bool messageWriteObject(const class DbmWriteObject* message);

	bool messageGetDataNames(const class DbmGetDataNames* message);

	bool messageRemoveAllData(const class DbmRemoveAllData* message);

	bool messageReadData(const class DbmReadData* message);

	bool messageWriteData(const class DbmWriteData* message);
};

	}
}

