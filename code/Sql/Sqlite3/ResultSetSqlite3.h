#pragma once

#include "Sql/IResultSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_SQLITE3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Semaphore;

	namespace sql
	{

/*! \brief Sqlite3 query/update result set.
 * \ingroup SQL
 */
class T_DLLCLASS ResultSetSqlite3 : public IResultSet
{
	T_RTTI_CLASS;

public:
	virtual bool next() override final;

	virtual int32_t getColumnCount() const override final;

	virtual std::wstring getColumnName(int32_t columnIndex) const override final;

	virtual ColumnType getColumnType(int32_t columnIndex) const override final;

	virtual int32_t getInt32(int32_t columnIndex) const override final;

	virtual int64_t getInt64(int32_t columnIndex) const override final;

	virtual float getFloat(int32_t columnIndex) const override final;

	virtual double getDouble(int32_t columnIndex) const override final;

	virtual std::wstring getString(int32_t columnIndex) const override final;

private:
	friend class ConnectionSqlite3;

	Semaphore& m_lock;
	void* m_stmt;

	ResultSetSqlite3(Semaphore& lock, void* stmt);

	virtual ~ResultSetSqlite3();
};

	}
}

