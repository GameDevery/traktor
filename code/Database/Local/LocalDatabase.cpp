#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/ConnectionString.h"
#include "Database/Local/Context.h"
#include "Database/Local/DefaultFileStore.h"
#include "Database/Local/LocalBus.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Local/LocalGroup.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalDatabase", 0, LocalDatabase, IProviderDatabase)

bool LocalDatabase::create(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"groupPath"))
		return false;

	std::wstring groupPath = connectionString.get(L"groupPath");

	Path groupPathA = FileSystem::getInstance().getAbsolutePath(groupPath);
	if (!FileSystem::getInstance().makeAllDirectories(groupPathA))
	{
		log::error << L"Unable to create physical group at \"" << groupPath << L"\"" << Endl;
		return false;
	}

	return open(connectionString);
}

bool LocalDatabase::open(const ConnectionString& connectionString)
{
	Ref< IFileStore > fileStore;

	if (!connectionString.have(L"groupPath"))
		return false;

	Path groupPath = FileSystem::getInstance().getAbsolutePath(connectionString.get(L"groupPath"));
	bool journal = connectionString.have(L"journal") ? parseString< bool >(connectionString.get(L"journal")) : true;
	bool binary = connectionString.have(L"binary") ? parseString< bool >(connectionString.get(L"binary")) : false;

	// Ensure group path exists.
	if (!FileSystem::getInstance().makeAllDirectories(groupPath))
	{
		log::error << L"Unable to ensure root group directory exist" << Endl;
		return false;
	}

	// Create file store.
	if (connectionString.have(L"fileStore"))
	{
		std::wstring fileStoreTypeName = connectionString.get(L"fileStore");

		const TypeInfo* fileStoreType = TypeInfo::find(fileStoreTypeName.c_str());
		if (fileStoreType)
			fileStore = checked_type_cast< IFileStore* >(fileStoreType->createInstance());

		if (!fileStore)
		{
			log::error << L"Unable to create file store from \"" << fileStoreTypeName << L"\"" << Endl;
			return false;
		}

		if (!fileStore->create(connectionString))
		{
			log::error << L"Unable to create file store  \"" << fileStoreTypeName << L"\"; using default file store" << Endl;
			fileStore = nullptr;
		}
	}

	// Fall back on default file store if none created.
	if (!fileStore)
	{
		fileStore = new DefaultFileStore();
		if (!fileStore->create(connectionString))
		{
			log::error << L"Unable to create file store" << Endl;
			return false;
		}
	}

	// Create context.
	m_context = Context(
		binary,
		fileStore
	);

	// Create event journal file.
	if (journal)
	{
		Path eventPath = groupPath.getPathName() + L"/Journal.bin";
		if (!FileSystem::getInstance().makeAllDirectories(eventPath.getPathOnly()))
		{
			log::error << L"Unable to ensure event journal directory exist." << Endl;
			return false;
		}

		m_bus = new LocalBus(eventPath.getPathName());
	}

	m_rootGroup = new LocalGroup(m_context, groupPath);
	return true;
}

void LocalDatabase::close()
{
	if (m_rootGroup)
		m_rootGroup = nullptr;

	if (m_bus)
	{
		m_bus->close();
		m_bus = nullptr;
	}

	if (m_context.getFileStore())
	{
		m_context.getFileStore()->destroy();
		m_context = Context();
	}
}

IProviderBus* LocalDatabase::getBus()
{
	return m_bus;
}

IProviderGroup* LocalDatabase::getRootGroup()
{
	return m_rootGroup;
}

	}
}
