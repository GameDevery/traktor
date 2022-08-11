#include <algorithm>
#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Database/Types.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/Transaction.h"
#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/ActionSetName.h"
#include "Database/Local/ActionRemove.h"
#include "Database/Local/ActionRemoveAllData.h"
#include "Database/Local/ActionWriteData.h"
#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/PhysicalAccess.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalInstance", LocalInstance, IProviderInstance)

LocalInstance::LocalInstance(Context& context, const Path& instancePath)
:	m_context(context)
,	m_instancePath(instancePath)
{
}

bool LocalInstance::internalCreateNew(const Guid& instanceGuid)
{
	m_transaction = new Transaction();
	if (!m_transaction->create(instanceGuid))
		return false;

	m_transactionName.clear();

	m_transaction->add(new ActionSetGuid(
		m_instancePath,
		instanceGuid,
		true
	));

	return true;
}

std::wstring LocalInstance::getPrimaryTypeName() const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	return instanceMeta ? instanceMeta->getPrimaryType() : L"";
}

bool LocalInstance::openTransaction()
{
	if (m_transaction)
		return false;

	m_transaction = new Transaction();
	if (!m_transaction->create(getGuid()))
	{
		m_transaction = nullptr;
		return false;
	}

	m_transactionName.clear();
	return true;
}

bool LocalInstance::commitTransaction()
{
	if (!m_transaction)
	{
		log::error << L"commitTransaction failed; no pending transaction." << Endl;
		return false;
	}

	if (!m_transaction->commit(m_context))
	{
		log::error << L"commitTransaction failed; commit failed." << Endl;
		return false;
	}

	if (!m_transactionName.empty())
		m_instancePath = m_instancePath.getPathOnly() + L"/" + m_transactionName;

	return true;
}

bool LocalInstance::closeTransaction()
{
	if (!m_transaction)
		return false;

	m_transaction->destroy();
	m_transaction = nullptr;

	return true;
}

std::wstring LocalInstance::getName() const
{
	return m_instancePath.getFileNameNoExtension();
}

bool LocalInstance::setName(const std::wstring& name)
{
	if (!m_transaction || name.empty())
		return false;

	if (getName() == name)
		return true;

	Ref< ActionSetName > action = new ActionSetName(m_instancePath, name);
	m_transaction->add(action);
	m_transactionName = name;

	return true;
}

Guid LocalInstance::getGuid() const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	return instanceMeta ? instanceMeta->getGuid() : Guid();
}

bool LocalInstance::setGuid(const Guid& guid)
{
	if (!m_transaction)
		return false;

	m_transaction->add(new ActionSetGuid(
		m_instancePath,
		guid,
		false
	));

	return true;
}

bool LocalInstance::getLastModifyDate(DateTime& outModifyDate) const
{
	Ref< File > instanceObjectFile = FileSystem::getInstance().get(getInstanceObjectPath(m_instancePath));
	if (instanceObjectFile)
	{
		outModifyDate = instanceObjectFile->getLastWriteTime();
		return true;
	}
	else
		return false;
}

uint32_t LocalInstance::getFlags() const
{
	return m_context.getFileStore()->flags(getInstanceObjectPath(m_instancePath));
}

bool LocalInstance::remove()
{
	if (!m_transaction)
		return false;

	m_transaction->add(new ActionRemove(
		m_instancePath
	));

	return true;
}

Ref< IStream > LocalInstance::readObject(const TypeInfo*& outSerializerType) const
{
	Ref< IStream > objectStream;

	// Get stream from transaction if pending.
	if (m_transaction)
	{
		RefArray< ActionWriteObject > actions;
		if (m_transaction->get< ActionWriteObject >(actions) > 0)
			objectStream = actions.back()->getReadStream();
	}

	// Open physical stream if no transaction.
	if (!objectStream)
	{
		Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
		objectStream = FileSystem::getInstance().open(instanceObjectPath, File::FmRead | File::FmMapped);
	}

	if (!objectStream)
		return nullptr;

	uint8_t head[5];
	if (objectStream->read(head, sizeof(head)) != sizeof(head))
	{
		objectStream->close();
		return nullptr;
	}

	objectStream->seek(IStream::SeekSet, 0);

	if (std::memcmp(head, "<?xml", sizeof(head)) == 0)
		outSerializerType = &type_of< xml::XmlDeserializer >();
	else
		outSerializerType = &type_of< BinarySerializer >();

	return objectStream;
}

Ref< IStream > LocalInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	if (!m_transaction)
		return nullptr;

	if (!m_context.preferBinary())
		outSerializerType = &type_of< xml::XmlSerializer >();
	else
		outSerializerType = &type_of< BinarySerializer >();

	Ref< ActionWriteObject > action = new ActionWriteObject(
		m_instancePath,
		primaryTypeName
	);
	m_transaction->add(action);

	return action->getWriteStream();
}

uint32_t LocalInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return 0;

	outDataNames.clear();
	for (const auto& blob : instanceMeta->getBlobs())
		outDataNames.push_back(blob);

	return uint32_t(outDataNames.size());
}

bool LocalInstance::getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const
{
	Ref< File > instanceDataFile = FileSystem::getInstance().get(getInstanceDataPath(m_instancePath, dataName));
	if (instanceDataFile)
	{
		outLastWriteTime = instanceDataFile->getLastWriteTime();
		return true;
	}
	else
		return false;
}

bool LocalInstance::removeAllData()
{
	if (!m_transaction)
		return false;

	Ref< ActionRemoveAllData > action = new ActionRemoveAllData(m_instancePath);
	m_transaction->add(action);

	return true;
}

Ref< IStream > LocalInstance::readData(const std::wstring& dataName) const
{
	// Read data from transaction if pending.
	if (m_transaction)
	{
		RefArray< ActionWriteData > actions;
		if (m_transaction->get< ActionWriteData >(actions) > 0)
			return actions.back()->getReadStream();
	}

	Path instanceDataPath = getInstanceDataPath(m_instancePath, dataName);
	return FileSystem::getInstance().open(instanceDataPath, File::FmRead | File::FmMapped);
}

Ref< IStream > LocalInstance::writeData(const std::wstring& dataName)
{
	if (!m_transaction)
		return nullptr;

	Ref< ActionWriteData > action = new ActionWriteData(m_instancePath, dataName);
	m_transaction->add(action);

	return action->getWriteStream();
}

	}
}
