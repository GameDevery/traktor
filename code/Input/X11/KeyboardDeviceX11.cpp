#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Input/X11/KeyboardDeviceX11.h"
#include "Input/X11/TypesX11.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceX11", KeyboardDeviceX11, InputDeviceX11)

KeyboardDeviceX11::KeyboardDeviceX11(Display* display, Window window, int deviceId)
:	m_display(display)
,	m_window(window)
,	m_deviceId(deviceId)
,	m_kbdesc(0)
,	m_connected(true)
{
	uint8_t mask[2] = { 0, 0 };
	XIEventMask evmask;

	evmask.mask = mask;
	evmask.mask_len = sizeof(mask);
	evmask.deviceid = m_deviceId;

	XISetMask(mask, XI_KeyPress);
	XISetMask(mask, XI_KeyRelease);

	XISelectEvents(m_display, m_window, &evmask, 1);
	XAutoRepeatOff(m_display);

	m_kbdesc = XkbGetKeyboard(m_display, XkbAllComponentsMask, XkbUseCoreKbd);

	resetState();
}

KeyboardDeviceX11::~KeyboardDeviceX11()
{
	setExclusive(false);
}

std::wstring KeyboardDeviceX11::getName() const
{
	return L"Standard Keyboard";
}

InputCategory KeyboardDeviceX11::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceX11::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceX11::getControlCount()
{
	return sizeof_array(m_keyStates);
}

std::wstring KeyboardDeviceX11::getControlName(int32_t control)
{
	KeySym ks = c_x11ControlKeys[control];
	if (ks != 0)
		return mbstows(XKeysymToString(ks));
	else
		return L"";
}

bool KeyboardDeviceX11::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceX11::isControlStable(int32_t control) const
{
	return false;
}

float KeyboardDeviceX11::getControlValue(int32_t control)
{
	if (m_connected)
		return m_keyStates[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool KeyboardDeviceX11::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceX11::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue || !c_x11ControlKeys[int32_t(controlType)])
		return false;

	control = int32_t(controlType);
	return true;
}

bool KeyboardDeviceX11::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();

	return true;
}

void KeyboardDeviceX11::resetState()
{
	std::memset(m_keyStates, 0, sizeof(m_keyStates));
	m_keyEvents.clear();
}

void KeyboardDeviceX11::readState()
{
	if (!m_connected)
		resetState();
}

bool KeyboardDeviceX11::supportRumble() const
{
	return false;
}

void KeyboardDeviceX11::setRumble(const InputRumble& /*rumble*/)
{
}

void KeyboardDeviceX11::setExclusive(bool exclusive)
{
	if (exclusive)
	{
		uint8_t mask[2] = { 0, 0 };
		XIEventMask evmask;

		evmask.mask = mask;
		evmask.mask_len = sizeof(mask);
		evmask.deviceid = m_deviceId;

		XISetMask(mask, XI_KeyPress);
		XISetMask(mask, XI_KeyRelease);

#if !defined(_DEBUG)
		XIGrabDevice(
			m_display,
			m_deviceId,
			m_window,
			CurrentTime,
			None,
			GrabModeAsync,
			GrabModeAsync,
			False,
			&evmask
		);
#endif
	}
	else
	{
#if !defined(_DEBUG)
		XIUngrabDevice(m_display, m_deviceId, CurrentTime);
#endif
	}
}

void KeyboardDeviceX11::consumeEvent(XEvent& evt)
{
	XIDeviceEvent* event = (XIDeviceEvent*)evt.xcookie.data;
	if (event->deviceid != m_deviceId)
		return;

	switch (event->evtype)
	{
	case XI_KeyPress:
		{
            KeySym ksym;
            unsigned int mods_rtrn;

            XkbTranslateKeyCode(
				m_kbdesc,
				event->detail,
				0,
				&mods_rtrn,
				&ksym
			);

            for (uint32_t i = 0; i < sizeof_array(c_x11ControlKeys); ++i)
            {
				if (c_x11ControlKeys[i] == ksym)
				{
					m_keyStates[i] = 0xff;
					break;
				}
			}
		}
		break;

	case XI_KeyRelease:
		{
            KeySym ksym;
            unsigned int mods_rtrn;

            XkbTranslateKeyCode(
				m_kbdesc,
				event->detail,
				0,
				&mods_rtrn,
				&ksym
			);

            for (uint32_t i = 0; i < sizeof_array(c_x11ControlKeys); ++i)
            {
				if (c_x11ControlKeys[i] == ksym)
				{
					m_keyStates[i] = 0x00;
					break;
				}
			}
		}
		break;

	default:
		log::info << L"Unknown event in device " << m_deviceId << Endl;
		break;
	}
}

	}
}
