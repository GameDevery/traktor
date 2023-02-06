/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/IInputDevice.h"
#include "Input/Binding/DeviceControl.h"

namespace traktor::input
{

std::wstring DeviceControl::getControlName() const
{
	if (m_device && m_device->isConnected())
		return m_device->getControlName(m_control);
	else
		return L"";
}

InputCategory DeviceControl::getCategory() const
{
	return m_category;
}

InputDefaultControlType DeviceControl::getControlType() const
{
	return m_controlType;
}

bool DeviceControl::isAnalogue() const
{
	return m_analogue;
}

int32_t DeviceControl::getIndex() const
{
	return m_index;
}

IInputDevice* DeviceControl::getDevice() const
{
	return m_device;
}

float DeviceControl::getRangeMin() const
{
	return m_rangeMin;
}

float DeviceControl::getRangeMax() const
{
	return m_rangeMax;
}

float DeviceControl::getPreviousValue() const
{
	return m_previousValue;
}

float DeviceControl::getCurrentValue() const
{
	return m_currentValue;
}

DeviceControl::DeviceControl()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_analogue(false)
,	m_index(0)
,	m_control(0)
,	m_rangeMin(0.0f)
,	m_rangeMax(0.0f)
,	m_previousValue(0.0f)
,	m_currentValue(0.0f)
{
}

}
