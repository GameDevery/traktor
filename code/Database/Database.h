#ifndef traktor_db_Database_H
#define traktor_db_Database_H

#include <map>
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Thread/Semaphore.h"
#include "Database/ConnectionString.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace db
	{

class IProviderDatabase;
class IProviderBus;
class Group;
class Instance;

/*! \brief Database engine.
 * \ingroup Database
 *
 * The Database class manages a local view of
 * a database provider.
 */
class T_DLLCLASS Database : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Open database from provider instance. */
	virtual bool open(IProviderDatabase* providerDatabase);

	/*! \brief Open database from connection string. */
	virtual bool open(const ConnectionString& connectionString);

	/*! \brief Create database from connection string. */
	virtual bool create(const ConnectionString& connectionString);

	virtual void close();

	/*! \brief Root group. */
	virtual Ref< Group > getRootGroup();

	/*! \brief Get group by path.
	 *
	 * \param groupPath Path to group.
	 * \return Group.
	 */
	virtual Ref< Group > getGroup(const std::wstring& groupPath);

	/*! \brief Create new group.
	 *
	 * \param groupPath Path to new group.
	 * \return New group.
	 */
	virtual Ref< Group > createGroup(const std::wstring& groupPath);

	/*! \brief Get instance by guid.
	 *
	 * \param instanceGuid Instance guid.
	 * \return Instance; null if no instance with given guid exist.
	 */
	virtual Ref< Instance > getInstance(const Guid& instanceGuid);

	/*! \brief Get instance by path.
	 *
	 * \param instancePath Path to instance.
	 * \param primaryType Optional primary type; if instance isn't of type then null is returned.
	 * \return Instance.
	 */
	virtual Ref< Instance > getInstance(const std::wstring& instancePath, const TypeInfo* primaryType = 0);

	/*! \brief Create new instance.
	 *
	 * \param instancePath Path to new instance.
	 * \param flags Create flags.
	 * \param guid Instance guid; null then guid is automatically created.
	 * \return New instance.
	 */
	virtual Ref< Instance > createInstance(const std::wstring& instancePath, uint32_t flags = CifDefault, const Guid* guid = 0);

	/*! \brief Get instance object by guid.
	 *
	 * \param guid Instance guid.
	 * \return Instance's object; null if no instance found.
	 */
	virtual Ref< ISerializable > getObjectReadOnly(const Guid& guid);

	template < typename T >
	Ref< T > getObjectReadOnly(const Guid& guid)
	{
		Ref< ISerializable > object = getObjectReadOnly(guid);
		return dynamic_type_cast< T* >(object);
	}

	/*! \brief Get event from bus.
	 *
	 * \note
	 * This method may flush database tree and should not be called
	 * from another thread if any instance or group is temporarily accessed.
	 *
	 * \param outEvent Event type.
	 * \param outEventId Event id, currently guid of affected instance.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote);

private:
	friend class Instance;

	Ref< IProviderDatabase > m_providerDatabase;
	Ref< IProviderBus > m_providerBus;
	Ref< Group > m_rootGroup;
	Semaphore m_lock;
	std::map< Guid, Ref< Instance > > m_instanceMap;

	/*! \brief Flush instance from cache. */
	void flushInstance(const Guid& instanceGuid);
};

	}
}

#endif	// traktor_db_Database_H
