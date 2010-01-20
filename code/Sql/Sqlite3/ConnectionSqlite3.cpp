#include <map>
#include <sqlite3.h>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"
#include "Sql/Sqlite3/ResultSetSqlite3.h"

namespace traktor
{
	namespace sql
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sql.ConnectionSqlite3", ConnectionSqlite3, IConnection)

ConnectionSqlite3::ConnectionSqlite3()
:	m_db(0)
{
}

bool ConnectionSqlite3::connect(const std::wstring& connectionString)
{
	std::vector< std::wstring > pairs;
	std::map< std::wstring, std::wstring > cs;

	if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
		return false;

	for (std::vector< std::wstring >::const_iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		size_t p = i->find(L'=');
		if (p == 0 || p == i->npos)
			return false;

		cs[trim(i->substr(0, p))] = i->substr(p + 1);
	}

	Path fileName = cs[L"fileName"];
	FileSystem::getInstance().makeAllDirectories(fileName.getPathOnly());

	log::debug << L"Using SQLite db \"" << fileName.getPathName() << L"\"" << Endl;

	sqlite3* db = 0;

	int err = sqlite3_open(
		wstombs(fileName.getPathName()).c_str(),
		&db
	);
	if (err != SQLITE_OK)
	{
		log::error << L"SQLite error (" << fileName.getPathName() << L"):" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return false;
	}

	m_db = (void*)db;
	return true;
}

void ConnectionSqlite3::disconnect()
{
	if (m_db)
	{
		sqlite3_close((sqlite3*)m_db);
		m_db = 0;
	}
}

Ref< IResultSet > ConnectionSqlite3::executeQuery(const std::wstring& query)
{
	sqlite3_stmt* stmt = 0;

	int err = sqlite3_prepare_v2(
		(sqlite3*)m_db,
		wstombs(query).c_str(),
		query.length(),
		&stmt,
		0
	);
	if (err != SQLITE_OK)
	{
		log::error << L"SQLite error:" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return 0;
	}

	err = sqlite3_step((sqlite3_stmt*)stmt);
	if (err != SQLITE_ROW)
	{
		sqlite3_finalize((sqlite3_stmt*)stmt);
		return 0;
	}

	return new ResultSetSqlite3((void*)stmt);
}

int32_t ConnectionSqlite3::executeUpdate(const std::wstring& update)
{
	sqlite3_stmt* stmt = 0;
	int err;

	err = sqlite3_prepare_v2(
		(sqlite3*)m_db,
		wstombs(update).c_str(),
		update.length(),
		&stmt,
		0
	);
	if (err != SQLITE_OK)
	{
		log::error << L"SQLite error:" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return 0;
	}

	err = sqlite3_step((sqlite3_stmt*)stmt);
	sqlite3_finalize((sqlite3_stmt*)stmt);

	return err == SQLITE_DONE ? 1 : 0;
}

bool ConnectionSqlite3::tableExists(const std::wstring& tableName)
{
	Ref< sql::IResultSet > rs = executeQuery(L"select count(*) from sqlite_master where name='" + tableName + L"'");
	return (rs && rs->getInt32(0) > 0);
}

	}
}
