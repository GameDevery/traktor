/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Feedback/EnvelopeFeedbackEvent.h"
#include "Spray/Feedback/EnvelopeFeedbackEventInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EnvelopeFeedbackEvent", EnvelopeFeedbackEvent, world::IEntityEvent)

EnvelopeFeedbackEvent::EnvelopeFeedbackEvent(const EnvelopeFeedbackEventData* data, IFeedbackManager* feedbackManager)
:	m_data(data)
,	m_feedbackManager(feedbackManager)
{
}

Ref< world::IEntityEventInstance > EnvelopeFeedbackEvent::createInstance(world::EntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	return new EnvelopeFeedbackEventInstance(m_data, m_feedbackManager);
}

	}
}
