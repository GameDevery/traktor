/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_OscillateFeedbackEventInstance_H
#define traktor_spray_OscillateFeedbackEventInstance_H

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace spray
	{

class OscillateFeedbackEventData;
class IFeedbackManager;

class OscillateFeedbackEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	OscillateFeedbackEventInstance(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void build(world::IWorldRenderer* worldRenderer) override final;

	virtual void cancel(world::CancelType when) override final;

private:
	const OscillateFeedbackEventData* m_data;
	IFeedbackManager* m_feedbackManager;
	float m_time;
};

	}
}

#endif	// traktor_spray_OscillateFeedbackEventInstance_H
