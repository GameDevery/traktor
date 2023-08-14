/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*! Input device base.
 * \ingroup Input
 *
 * Implementations of this class is created
 * through IInputDriver class.
 */
class T_DLLCLASS IInputDevice : public Object
{
	T_RTTI_CLASS;

public:
	enum KeyEventType
	{
		KtDown,
		KtUp,
		KtCharacter
	};

	struct KeyEvent
	{
		KeyEventType type;
		union
		{
			uint32_t keyCode;
			wchar_t character;
		};
	};

	/*! Human readable name of device provided by system.
	 *
	 * \return Human readable name of device.
	 */
	virtual std::wstring getName() const = 0;

	/*! Get device category.
	 *
	 * Devices are identified by category
	 * such as keyboard, joystick etc.
	 *
	 * \return Device category.
	 */
	virtual InputCategory getCategory() const = 0;

	/*! Query device if it's currently connected.
	 *
	 * Devices might become disconnected during
	 * input system update and thus should
	 * be queried first in order to ensure
	 * the device is still connected.
	 *
	 * \return True if device is connected.
	 */
	virtual bool isConnected() const = 0;

	/*! Number of controls the device support.
	 *
	 * \return Number of controls.
	 */
	virtual int32_t getControlCount() = 0;

	/*! Get human readable name of control.
	 *
	 * \param control Control identifier (0 - #controls-1).
	 * \return Human readable name of control.
	 */
	virtual std::wstring getControlName(int32_t control) = 0;

	/*! Check if control is analogue.
	 *
	 * Digital controls always return either 0 or 1
	 * even if the return type of getControlValue is float.
	 *
	 * \param control Control identifier (0 - #controls-1).
	 * \return True if control is analogue.
	 */
	virtual bool isControlAnalogue(int32_t control) const = 0;

	/*! Check if control is stable.
	 *
	 * Stable controls have a defined value when
	 * no user interacts with the control.
	 *
	 * \param control Control identifier (0 - #controls-1).
	 * \return True if control is stable.
	 */
	virtual bool isControlStable(int32_t control) const = 0;

	/*! Get current value of a control.
	 *
	 * \param control Control identifier (0 - #controls-1).
	 * \return Control value.
	 */
	virtual float getControlValue(int32_t control) = 0;

	/*! Get range of a control.
	 *
	 * \param control Control identifier (0 - #controls-1).
	 * \param outMin Minimum value.
	 * \param outMax Maximum value.
	 * \return True if range is valid.
	 */
	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const = 0;

	/*! Get control identifier of a default control type.
	 *
	 * \param controlType Default control type.
	 * \param analogue If control should be used as analogue.
	 * \param control Output control identifier.
	 * \return True if device support control type and have successfully mapped identifier.
	 */
	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const = 0;

	/*! Get keyboard event.
	 *
	 * \param outEvent Returning event.
	 * \return True if event returned.
	 */
	virtual bool getKeyEvent(KeyEvent& outEvent) = 0;

	/*! Reset device state.
	 *
	 * Device "state" are implementation specific but
	 * should be reset into some sort of idle state which
	 * represent "no buttons held" etc.
	 */
	virtual void resetState() = 0;

	/*! Read device state.
	 *
	 * Update device internal state; state should be preserved
	 * and thus getControlValue should return same value
	 * until next readState is called; ie no async state update allowed.
	 */
	virtual void readState() = 0;

	/*! Check to see if device support rumble.
	 *
	 * \return True if device support rumble.
	 */
	virtual bool supportRumble() const = 0;

	/*! Set device rumble parameters.
	 *
	 * \param rumble Rumble parameter configuration.
	 */
	virtual void setRumble(const InputRumble& rumble) = 0;

	/*! Set exclusive use of device.
	 *
	 * \param exclusive If exclusive use is desired.
	 */
	virtual void setExclusive(bool exclusive) = 0;
};

}
