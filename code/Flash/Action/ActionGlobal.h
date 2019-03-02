#pragma once

#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief ActionScript global object.
 * \ingroup Flash
 */
class T_DLLCLASS ActionGlobal : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionGlobal(ActionContext* context);

private:
	void Global_ASSetPropFlags(CallArgs& ca);

	void Global_escape(CallArgs& ca);

	void Global_isNaN(CallArgs& ca);
};

	}
}

