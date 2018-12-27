/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SoundEvent_H
#define traktor_spray_SoundEvent_H

#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;
class Sound;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class SoundEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	SoundEvent(
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< sound::Sound >& sound,
		bool positional,
		bool follow,
		bool autoStopFar
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	sound::ISoundPlayer* m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
};

	}
}

#endif	// traktor_spray_SoundEvent_H
