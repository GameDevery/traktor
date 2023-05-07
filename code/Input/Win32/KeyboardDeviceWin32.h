/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/CircularVector.h"
#include "Input/IInputDevice.h"
#include "Input/Win32/TypesWin32.h"

namespace traktor::input
{

class KeyboardDeviceWin32 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit KeyboardDeviceWin32(HWND hWnd);

	virtual ~KeyboardDeviceWin32();

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual bool isControlStable(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual bool getKeyEvent(KeyEvent& outEvent);

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

	virtual void setExclusive(bool exclusive);

private:
	friend class InputDriverWin32;

	bool m_connected;
	HWND m_hWnd;
	WNDPROC m_pWndProc;
	CircularVector< KeyEvent, 16 > m_keyEvents;
	uint8_t m_keyStates[sizeof_array(c_vkControlKeys)];

	static LRESULT WINAPI wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}
