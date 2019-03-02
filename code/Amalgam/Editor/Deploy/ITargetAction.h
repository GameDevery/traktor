#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Abstract target action class.
 * \ingroup Amalgam
 */
class T_DLLCLASS ITargetAction : public Object
{
	T_RTTI_CLASS;

public:
	struct IProgressListener : public IRefCount
	{
		virtual void notifyTargetActionProgress(int32_t currentStep, int32_t maxStep) = 0;
	};

	virtual bool execute(IProgressListener* progressListener) = 0;
};

	}
}

