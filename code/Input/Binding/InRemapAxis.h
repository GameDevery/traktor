/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*!
 * \ingroup Input
 */
class T_DLLCLASS InRemapAxis : public IInputNode
{
	T_RTTI_CLASS;

public:
	InRemapAxis();

	explicit InRemapAxis(IInputNode* source, float limitMin, float limitMax, float outputMin, float outputMid, float outputMax);

	virtual Ref< Instance > createInstance() const override final;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class InRemapAxisTraits;

	Ref< IInputNode > m_source;
	float m_limit[2];
	float m_output[3];
};

}
