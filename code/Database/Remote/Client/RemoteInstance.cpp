#include "Database/Remote/Client/RemoteInstance.h"
#include "Database/Remote/Client/RemoteStream.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/DbmGetInstancePrimaryType.h"
#include "Database/Remote/Messages/DbmOpenTransaction.h"
#include "Database/Remote/Messages/DbmCommitTransaction.h"
#include "Database/Remote/Messages/DbmCloseTransaction.h"
#include "Database/Remote/Messages/DbmGetInstanceName.h"
#include "Database/Remote/Messages/DbmSetInstanceName.h"
#include "Database/Remote/Messages/DbmGetInstanceGuid.h"
#include "Database/Remote/Messages/DbmSetInstanceGuid.h"
#include "Database/Remote/Messages/DbmRemoveInstance.h"
#include "Database/Remote/Messages/DbmReadObject.h"
#include "Database/Remote/Messages/DbmWriteObject.h"
#include "Database/Remote/Messages/DbmGetDataNames.h"
#include "Database/Remote/Messages/DbmReadData.h"
#include "Database/Remote/Messages/DbmWriteData.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgStringArrayResult.h"
#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"
#include "Database/Remote/Messages/DbmWriteObjectResult.h"
#include "Core/Io/BufferedStream.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteInstance", RemoteInstance, IProviderInstance)

RemoteInstance::RemoteInstance(RemoteConnection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteInstance::~RemoteInstance()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

std::wstring RemoteInstance::getPrimaryTypeName() const
{
	Ref< MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstancePrimaryType(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::openTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmOpenTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::commitTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCommitTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::closeTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCloseTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

std::wstring RemoteInstance::getName() const
{
	Ref< MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstanceName(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::setName(const std::wstring& name)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceName(m_handle, name));
	return result ? result->getStatus() == StSuccess : false;
}

Guid RemoteInstance::getGuid() const
{
	Ref< MsgGuidResult > result = m_connection->sendMessage< MsgGuidResult >(DbmGetInstanceGuid(m_handle));
	return result ? result->get() : Guid();
}

bool RemoteInstance::setGuid(const Guid& guid)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceGuid(m_handle, guid));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::remove()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveInstance(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Ref< IStream > RemoteInstance::readObject(const TypeInfo*& outSerializerType)
{
	Ref< DbmReadObjectResult > result = m_connection->sendMessage< DbmReadObjectResult >(DbmReadObject(m_handle));
	if (!result)
		return 0;

	outSerializerType = TypeInfo::find(result->getSerializerTypeName());
	if (!outSerializerType)
		return 0;

	Ref< RemoteStream > remoteStream = new RemoteStream(m_connection, result->getHandle());
	return new BufferedStream(remoteStream);
}

Ref< IStream > RemoteInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	Ref< DbmWriteObjectResult > result = m_connection->sendMessage< DbmWriteObjectResult >(DbmWriteObject(m_handle, primaryTypeName));
	if (!result)
		return 0;

	outSerializerType = TypeInfo::find(result->getSerializerTypeName());
	if (!outSerializerType)
		return 0;

	Ref< RemoteStream > remoteStream = new RemoteStream(m_connection, result->getHandle());
	return new BufferedStream(remoteStream);
}

uint32_t RemoteInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	Ref< MsgStringArrayResult > result = m_connection->sendMessage< MsgStringArrayResult >(DbmGetDataNames(m_handle));
	if (!result)
		return 0;

	outDataNames = result->get();
	return uint32_t(outDataNames.size());
}

Ref< IStream > RemoteInstance::readData(const std::wstring& dataName)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmReadData(m_handle, dataName));
	if (!result)
		return 0;

	Ref< RemoteStream > remoteStream = new RemoteStream(m_connection, result->get());
	return new BufferedStream(remoteStream);
}

Ref< IStream > RemoteInstance::writeData(const std::wstring& dataName)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmWriteData(m_handle, dataName));
	if (!result)
		return 0;

	Ref< RemoteStream > remoteStream = new RemoteStream(m_connection, result->get());
	return new BufferedStream(remoteStream);
}

	}
}
