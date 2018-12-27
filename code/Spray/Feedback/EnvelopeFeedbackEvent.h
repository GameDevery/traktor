/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_EnvelopeFeedbackEvent_H
#define traktor_spray_EnvelopeFeedbackEvent_H

#include "World/IEntityEvent.h"

namespace traktor
{
	namespace spray
	{

class EnvelopeFeedbackEventData;
class IFeedbackManager;

class EnvelopeFeedbackEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	EnvelopeFeedbackEvent(const EnvelopeFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual Ref< world::IEntityEventInstance > createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	Ref< const EnvelopeFeedbackEventData > m_data;
	IFeedbackManager* m_feedbackManager;
};

	}
}

#endif	// traktor_spray_EnvelopeFeedbackEvent_H
