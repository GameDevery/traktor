#include "Runtime/Target/ScriptDebuggerBreakpoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerBreakpoint", 0, ScriptDebuggerBreakpoint, ISerializable)

ScriptDebuggerBreakpoint::ScriptDebuggerBreakpoint()
:	m_add(false)
,	m_lineNumber(0)
{
}

ScriptDebuggerBreakpoint::ScriptDebuggerBreakpoint(bool add, const Guid& scriptId, uint32_t lineNumber)
:	m_add(add)
,	m_scriptId(scriptId)
,	m_lineNumber(lineNumber)
{
}

void ScriptDebuggerBreakpoint::serialize(ISerializer& s)
{
	s >> Member< bool >(L"add", m_add);
	s >> Member< Guid >(L"scriptId", m_scriptId);
	s >> Member< uint32_t >(L"lineNumber", m_lineNumber);
}

	}
}
