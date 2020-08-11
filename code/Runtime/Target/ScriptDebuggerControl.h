#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Control debugger on running target.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerControl : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum Action
	{
		AcStatus = 0,
		AcBreak = 1,
		AcContinue = 2,
		AcStepInto = 3,
		AcStepOver = 4,
		AcCaptureStack = 5,
		AcCaptureLocals = 6,
		AcCaptureObject = 7,
		AcCaptureBreadcrumbs = 8
	};

	ScriptDebuggerControl() = default;

	explicit ScriptDebuggerControl(Action action);

	explicit ScriptDebuggerControl(Action action, uint32_t param);

	Action getAction() const { return m_action; }

	uint32_t getParam() const { return m_param; }

	virtual void serialize(ISerializer& s) override final;

private:
	Action m_action = AcBreak;
	uint32_t m_param = 0;
};

	}
}

