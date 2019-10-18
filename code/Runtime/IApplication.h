#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
	#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

class IEnvironment;
class IStateManager;

/*! Runtime application.
 * \ingroup Runtime
 *
 * This interface represent the running
 * instance of the application.
 */
class T_DLLCLASS IApplication : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get runtime environment.
	 *
	 * \return Runtime environment.
	 */
	virtual IEnvironment* getEnvironment() = 0;

	/*! \brief Get state manager.
	 *
	 * \return State manager.
	 */
	virtual IStateManager* getStateManager() = 0;
};

	}
}

