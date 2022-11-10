/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Input/Binding/InEnvelope.h"
#include "Input/Editor/InEnvelopeTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InEnvelopeTraits::getHeader(const IInputNode* node) const
{
	return L"Envelope";
}

std::wstring InEnvelopeTraits::getDescription(const IInputNode* node) const
{
	return L"Envelope";
}

Ref< IInputNode > InEnvelopeTraits::createNode() const
{
	return new InEnvelope();
}

void InEnvelopeTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InEnvelope* ie = checked_type_cast< const InEnvelope*, false >(node);
	outInputNodes[L"Input"] = ie->m_source;
}

void InEnvelopeTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InEnvelope* ie = checked_type_cast< InEnvelope*, false >(node);
	ie->m_source = sourceNode;
}

void InEnvelopeTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InEnvelope* ie = checked_type_cast< InEnvelope*, false >(node);
	ie->m_source = 0;
}

	}
}
