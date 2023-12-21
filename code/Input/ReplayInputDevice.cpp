/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/ReplayInputDevice.h"
#include "Input/RecordInputScript.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ReplayInputDevice", ReplayInputDevice, IInputDevice)

ReplayInputDevice::ReplayInputDevice(IInputDevice* inputDevice, const RecordInputScript* inputScript, bool loop)
:	m_inputDevice(inputDevice)
,	m_inputScript(inputScript)
,	m_loop(loop)
,	m_frame(0)
{
}

std::wstring ReplayInputDevice::getName() const
{
	return m_inputDevice->getName();
}

InputCategory ReplayInputDevice::getCategory() const
{
	return m_inputDevice->getCategory();
}

bool ReplayInputDevice::isConnected() const
{
	return true;
}

int32_t ReplayInputDevice::getControlCount()
{
	return m_inputDevice->getControlCount();
}

std::wstring ReplayInputDevice::getControlName(int32_t control)
{
	return m_inputDevice->getControlName(control);
}

bool ReplayInputDevice::isControlAnalogue(int32_t control) const
{
	return m_inputDevice->isControlAnalogue(control);
}

float ReplayInputDevice::getControlValue(int32_t control)
{
	return m_inputScript->getInputValue(m_frame, control);
}

bool ReplayInputDevice::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return m_inputDevice->getControlRange(control, outMin, outMax);
}

bool ReplayInputDevice::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	return m_inputDevice->getDefaultControl(controlType, analogue, control);
}

bool ReplayInputDevice::getKeyEvent(KeyEvent& outEvent)
{
	return m_inputDevice->getKeyEvent(outEvent);
}

void ReplayInputDevice::resetState()
{
	m_frame = 0;
}

void ReplayInputDevice::readState()
{
	m_frame++;
	if (m_loop && m_frame > m_inputScript->getLastFrame())
		m_frame = 0;
}

bool ReplayInputDevice::supportRumble() const
{
	return false;
}

void ReplayInputDevice::setRumble(const InputRumble& rumble)
{
}

}
