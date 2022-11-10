/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Target/ScriptDebuggerLocals.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Script/Variable.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerLocals", 0, ScriptDebuggerLocals, ISerializable)

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
