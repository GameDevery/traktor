#pragma once

#include "Core/Guid.h"
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

/*! Add or remove script breakpoint on running target.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerBreakpoint : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerBreakpoint() = default;

	ScriptDebuggerBreakpoint(bool add, const Guid& scriptId, uint32_t lineNumber);

	bool shouldAdd() const { return m_add; }

	const Guid& getScriptId() const { return m_scriptId; }

	uint32_t getLineNumber() const { return m_lineNumber; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_add = false;
	Guid m_scriptId;
	uint32_t m_lineNumber = 0;
};

	}
}

