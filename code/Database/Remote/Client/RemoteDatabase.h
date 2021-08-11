#pragma once

#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_CLIENT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class RemoteConnection;

/*! Remote database provider.
 * \ingroup Database
 */
class T_DLLCLASS RemoteDatabase : public IProviderDatabase
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) override final;

	virtual bool open(const ConnectionString& connectionString) override final;

	virtual void close() override final;

	virtual IProviderBus* getBus() override final;

	virtual IProviderGroup* getRootGroup() override final;

private:
	Ref< RemoteConnection > m_connection;
	Ref< IProviderBus > m_bus;
	Ref< IProviderGroup > m_rootGroup;
};

	}
}

