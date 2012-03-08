#ifndef traktor_amalgam_OnlineServer_H
#define traktor_amalgam_OnlineServer_H

#include "Amalgam/IOnlineServer.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace amalgam
	{

class OnlineServer : public IOnlineServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, db::Database* db);

	void destroy();

	int32_t reconfigure(const PropertyGroup* settings);

	virtual online::ISessionManager* getSessionManager();

private:
	Ref< online::ISessionManager > m_sessionManager;
};

	}
}

#endif	// traktor_amalgam_OnlineServer_H
