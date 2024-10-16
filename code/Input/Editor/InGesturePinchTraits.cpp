/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InGesturePinch.h"
#include "Input/Editor/InGesturePinchTraits.h"

namespace traktor::input
{

std::wstring InGesturePinchTraits::getHeader(const IInputNode* node) const
{
	return L"Gesture Pinch";
}

std::wstring InGesturePinchTraits::getDescription(const IInputNode* node) const
{
	const InGesturePinch* inGesturePinch = checked_type_cast< const InGesturePinch*, false >(node);
	switch (inGesturePinch->m_direction)
	{
	case InGesturePinch::PdAny:
		return L"";
	case InGesturePinch::PdX:
		return L"Horizontal";
	case InGesturePinch::PdY:
		return L"Vertical";
	default:
		return L"";
	}
}

Ref< IInputNode > InGesturePinchTraits::createNode() const
{
	return new InGesturePinch();
}

void InGesturePinchTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InGesturePinch* inGesturePinch = checked_type_cast< const InGesturePinch*, false >(node);
	outInputNodes[L"Active"] = inGesturePinch->m_sourceActive;
	outInputNodes[L"X1"] = inGesturePinch->m_sourceX1;
	outInputNodes[L"Y1"] = inGesturePinch->m_sourceY1;
	outInputNodes[L"X2"] = inGesturePinch->m_sourceX2;
	outInputNodes[L"Y2"] = inGesturePinch->m_sourceY2;
}

void InGesturePinchTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InGesturePinch* inGesturePinch = checked_type_cast< InGesturePinch*, false >(node);
	if (inputName == L"Active")
		inGesturePinch->m_sourceActive = sourceNode;
	else if (inputName == L"X1")
		inGesturePinch->m_sourceX1 = sourceNode;
	else if (inputName == L"Y1")
		inGesturePinch->m_sourceY1 = sourceNode;
	else if (inputName == L"X2")
		inGesturePinch->m_sourceX2 = sourceNode;
	else if (inputName == L"Y2")
		inGesturePinch->m_sourceY2 = sourceNode;
}

void InGesturePinchTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InGesturePinch* inGesturePinch = checked_type_cast< InGesturePinch*, false >(node);
	if (inputName == L"Active")
		inGesturePinch->m_sourceActive = nullptr;
	else if (inputName == L"X1")
		inGesturePinch->m_sourceX1 = nullptr;
	else if (inputName == L"Y1")
		inGesturePinch->m_sourceY1 = nullptr;
	else if (inputName == L"X2")
		inGesturePinch->m_sourceX2 = nullptr;
	else if (inputName == L"Y2")
		inGesturePinch->m_sourceY2 = nullptr;
}

}
