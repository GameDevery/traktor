/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Input/RumbleEffectPlayer.h"
#include "Input/RumbleEffect.h"
#include "Input/IInputDevice.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RumbleEffectPlayer", RumbleEffectPlayer, Object)

void RumbleEffectPlayer::play(RumbleEffect* effect, IInputDevice* targetDevice)
{
	T_ASSERT(targetDevice->supportRumble());
	m_playingEffects.push_back(PlayingEffect(effect, targetDevice, m_totalTime));
}

void RumbleEffectPlayer::stop(RumbleEffect* effect, IInputDevice* targetDevice)
{
	std::list< PlayingEffect >::iterator i = std::find(m_playingEffects.begin(), m_playingEffects.end(), PlayingEffect(effect, targetDevice, 0.0f));
	if (i != m_playingEffects.end())
		m_playingEffects.erase(i);
}

void RumbleEffectPlayer::stopAll()
{
	InputRumble zeroRumble = { 0.0f, 0.0f };
	for (std::list< PlayingEffect >::iterator i = m_playingEffects.begin(); i != m_playingEffects.end(); i++)
		i->targetDevice->setRumble(zeroRumble);
	m_playingEffects.clear();
}

void RumbleEffectPlayer::update(float deltaTime)
{
	for (std::list< PlayingEffect >::iterator i = m_playingEffects.begin(); i != m_playingEffects.end(); )
	{
		float effectTime = m_totalTime - i->attachedTime;
		if (effectTime <= i->effect->getDuration() && i->targetDevice->isConnected())
		{
			InputRumble rumble;
			i->effect->getRumble(effectTime, rumble);
			i->targetDevice->setRumble(rumble);
			i++;
		}
		else
		{
			InputRumble zeroRumble = { 0.0f, 0.0f };
			i->targetDevice->setRumble(zeroRumble);
			i = m_playingEffects.erase(i);
		}
	}
	m_totalTime += deltaTime;
}

RumbleEffectPlayer::PlayingEffect::PlayingEffect(RumbleEffect* effect_, IInputDevice* targetDevice_, float attachedTime_)
:	effect(effect_)
,	targetDevice(targetDevice_)
,	attachedTime(attachedTime_)
{
}

bool RumbleEffectPlayer::PlayingEffect::operator == (const PlayingEffect& other) const
{
	return effect == other.effect && targetDevice == other.targetDevice;
}

}
