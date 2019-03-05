#include "Amalgam/Target/ScriptDebuggerLocals.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Script/Variable.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerLocals", 0, ScriptDebuggerLocals, ISerializable)

ScriptDebuggerLocals::ScriptDebuggerLocals()
{
}

ScriptDebuggerLocals::ScriptDebuggerLocals(const RefArray< script::Variable >& locals)
:	m_locals(locals)
{
}

void ScriptDebuggerLocals::serialize(ISerializer& s)
{
	s >> MemberRefArray< script::Variable >(L"locals", m_locals);
}

	}
}
