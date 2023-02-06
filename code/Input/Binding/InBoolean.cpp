/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Input/Binding/InBoolean.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor::input
{
	namespace
	{

struct InBooleanInstance : public RefCountImpl< IInputNode::Instance >
{
	RefArray< IInputNode::Instance > sourceInstance;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InBoolean", 0, InBoolean, IInputNode)

InBoolean::InBoolean()
:	m_op(OpAnd)
{
}

InBoolean::InBoolean(
	IInputNode* source1,
	IInputNode* source2,
	Operator op
)
:	m_source(2)
,	m_op(op)
{
	m_source[0] = source1;
	m_source[1] = source2;
}

Ref< IInputNode::Instance > InBoolean::createInstance() const
{
	Ref< InBooleanInstance > instance = new InBooleanInstance();
	for (auto source : m_source)
		instance->sourceInstance.push_back(source->createInstance());
	return instance;
}

float InBoolean::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InBooleanInstance* ibi = static_cast< InBooleanInstance* >(instance);

	bool result = false;
	for (uint32_t i = 0; i < uint32_t(m_source.size()); ++i)
	{
		const bool value = asBoolean(m_source[i]->evaluate(ibi->sourceInstance[i], valueSet, T, dT));

		switch (m_op)
		{
		case OpNot:
			result = !value;
			break;

		case OpAnd:
			result &= value;
			break;

		case OpOr:
			result |= value;
			break;

		case OpXor:
			result ^= value;
			break;
		}
	}

	return asFloat(result);
}

void InBoolean::serialize(ISerializer& s)
{
	const MemberEnum< Operator >::Key c_Operator_Keys[] =
	{
		{ L"OpNot", OpNot },
		{ L"OpAnd", OpAnd },
		{ L"OpOr", OpOr },
		{ L"OpXor", OpXor },
		{ 0 }
	};

	s >> MemberRefArray< IInputNode >(L"source", m_source);
	s >> MemberEnum< Operator >(L"op", m_op, c_Operator_Keys);
}

}
