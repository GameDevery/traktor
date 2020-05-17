#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XKB.h>
#include "Core/Containers/CircularVector.h"
#include "Input/X11/InputDeviceX11.h"

namespace traktor
{
	namespace input
	{

class KeyboardDeviceX11 : public InputDeviceX11
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceX11(Display* display, Window window, int deviceId);

	virtual ~KeyboardDeviceX11();

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual bool isControlStable(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

	virtual void setExclusive(bool exclusive) override final;

	virtual void consumeEvent(XEvent& evt) override final;

	virtual void setFocus(bool focus) override final;

private:
	Display* m_display;
	Window m_window;
	int m_deviceId;
	XkbDescPtr m_kbdesc;
	bool m_connected;
	bool m_exclusive;
	bool m_focus;
	bool m_haveGrab;
	CircularVector< KeyEvent, 16 > m_keyEvents;
	uint8_t m_keyStates[256];
};

	}
}

