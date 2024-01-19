/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Player/SoundListener.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundListener", SoundListener, ISoundListener)

void SoundListener::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Transform SoundListener::getTransform() const
{
	return m_transform;
}

}
