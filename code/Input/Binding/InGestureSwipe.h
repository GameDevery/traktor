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
class T_DLLCLASS InGestureSwipe : public IInputNode
{
	T_RTTI_CLASS;

public:
	enum SwipeDirection
	{
		SdUp,
		SdDown,
		SdLeft,
		SdRight
	};

	InGestureSwipe();

	explicit InGestureSwipe(
		IInputNode* sourceActive,
		IInputNode* sourceX,
		IInputNode* sourceY,
		SwipeDirection direction
	);

	virtual Ref< Instance > createInstance() const override final;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class InGestureSwipeTraits;

	Ref< IInputNode > m_sourceActive;
	Ref< IInputNode > m_sourceX;
	Ref< IInputNode > m_sourceY;
	SwipeDirection m_direction;
};

}
