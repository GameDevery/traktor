#pragma once

#include "Amalgam/Run/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_RUN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptManager;

	}

	namespace amalgam
	{

/*! \brief Script server.
 * \ingroup Amalgam
 *
 * "Script.Library"	- Script library.
 * "Script.Type"	- Script manager type.
 */
class T_DLLCLASS IScriptServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual script::IScriptManager* getScriptManager() = 0;
};

	}
}

