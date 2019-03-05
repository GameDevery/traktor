#pragma once

#include "Amalgam/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class ISessionManager;

	}

	namespace amalgam
	{

/*! \brief Online server.
 * \ingroup Amalgam
 */
class T_DLLCLASS IOnlineServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual online::ISessionManager* getSessionManager() = 0;
};

	}
}

