/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/NodeMovedEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NodeMovedEvent", NodeMovedEvent, Event)

NodeMovedEvent::NodeMovedEvent(EventSubject* sender, Node* node)
:	Event(sender)
,	m_node(node)
{
}

Node* NodeMovedEvent::getNode() const
{
	return m_node;
}

	}
}
