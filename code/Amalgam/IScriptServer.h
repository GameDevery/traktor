#ifndef traktor_amalgam_IScriptServer_H
#define traktor_amalgam_IScriptServer_H

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
	namespace script
	{

class IScriptManager;

	}

	namespace amalgam
	{

/*! \brief Script server.
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

#endif	// traktor_amalgam_IScriptServer_H
