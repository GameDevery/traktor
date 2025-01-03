/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControlManager.h"
#include "Input/Binding/ControlInputSource.h"
#include "Input/Binding/ControlInputSourceData.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ControlInputSource", ControlInputSource, IInputSource)

ControlInputSource::ControlInputSource(const ControlInputSourceData* data, DeviceControlManager* deviceControlManager)
:	m_data(data)
,	m_deviceControlManager(deviceControlManager)
{
}

std::wstring ControlInputSource::getDescription() const
{
	return L"";
}

void ControlInputSource::prepare(float T, float dT)
{
}

float ControlInputSource::read(float T, float dT)
{
	const InputCategory category = m_data->getCategory();
	const int32_t index = m_data->getIndex();

	InputSystem* inputSystem = m_deviceControlManager->getInputSystem();
	T_ASSERT(inputSystem);

	if (index < 0)
	{
		const int32_t deviceCount = inputSystem->getDeviceCount(category, false);
		if (m_data->getControlQuery() == ControlInputSourceData::CqMatchingDevice)
			return deviceCount > 0 ? 1.0f : 0.0f;
		else
		{
			for (int32_t i = 0; i < deviceCount; ++i)
			{
				const IInputDevice* inputDevice = inputSystem->getDevice(category, i, false);
				T_ASSERT(inputDevice);

				if (inputDevice->isConnected())
					return 1.0f;
			}
			return 0.0f;
		}
	}
	else
	{
		const IInputDevice* inputDevice = inputSystem->getDevice(category, index, false);
		if (!inputDevice)
			return 0.0f;

		if (m_data->getControlQuery() == ControlInputSourceData::CqMatchingDevice)
			return 1.0f;
		else
			return inputDevice->isConnected() ? 1.0f : 0.0f;
	}

	return 0.0f;
}

}
