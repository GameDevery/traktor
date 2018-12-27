/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_OscillateFeedbackEvent_H
#define traktor_spray_OscillateFeedbackEvent_H

#include "World/IEntityEvent.h"

namespace traktor
{
	namespace spray
	{

class OscillateFeedbackEventData;
class IFeedbackManager;

class OscillateFeedbackEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	OscillateFeedbackEvent(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual Ref< world::IEntityEventInstance > createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	Ref< const OscillateFeedbackEventData > m_data;
	IFeedbackManager* m_feedbackManager;
};

	}
}

#endif	// traktor_spray_OscillateFeedbackEvent_H
