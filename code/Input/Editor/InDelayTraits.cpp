/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Input/Binding/InDelay.h"
#include "Input/Editor/InDelayTraits.h"

namespace traktor::input
{

std::wstring InDelayTraits::getHeader(const IInputNode* node) const
{
	return L"Delay";
}

std::wstring InDelayTraits::getDescription(const IInputNode* node) const
{
	const InDelay* inDelay = checked_type_cast< const InDelay*, false >(node);
	return L"Delay " + toString(inDelay->m_delay);
}

Ref< IInputNode > InDelayTraits::createNode() const
{
	return new InDelay();
}

void InDelayTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InDelay* inDelay = checked_type_cast< const InDelay*, false >(node);
	outInputNodes[L"Input"] = inDelay->m_source;
}

void InDelayTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InDelay* inDelay = checked_type_cast< InDelay*, false >(node);
	inDelay->m_source = sourceNode;
}

void InDelayTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InDelay* inDelay = checked_type_cast< InDelay*, false >(node);
	inDelay->m_source = nullptr;
}

}
