#pragma once

#include "Core/Thread/Semaphore.h"
#include "Sql/IConnection.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_SQLITE3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

/*! Sqlite3 database connection.
 * \ingroup SQL
 *
 * Connection string definition:
 * {key=value};*
 *
 * fileName	Database filename.
 */
class T_DLLCLASS ConnectionSqlite3 : public IConnection
{
	T_RTTI_CLASS;

public:
	ConnectionSqlite3();

	virtual bool connect(const std::wstring& connectionString) override final;

	virtual void disconnect() override final;

	virtual Ref< IResultSet > executeQuery(const std::wstring& query) override final;

	virtual int32_t executeUpdate(const std::wstring& update) override final;

	virtual int32_t lastInsertId() override final;

	virtual bool tableExists(const std::wstring& tableName) override final;

private:
	Semaphore m_lock;
	void* m_db;
};

	}
}

