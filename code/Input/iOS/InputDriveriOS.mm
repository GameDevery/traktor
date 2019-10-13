#include "Core/Log/Log.h"
#include "Input/iOS/GamepadDeviceiOS.h"
#include "Input/iOS/InputDriveriOS.h"
#include "Input/iOS/TouchDeviceiOS.h"
#include "Input/iOS/MouseDeviceiOS.h"

namespace traktor
{
	namespace input
	{

class InputDriveriOSImpl
:	public Object
,	public ITouchViewCallback
{
public:
	bool create(void* nativeWindowHandle);

	int getDeviceCount();

	Ref< IInputDevice > getDevice(int index);

	// ITouchViewCallback

	virtual void touchesBegan(NSSet* touches, UIEvent* event);

	virtual void touchesMoved(NSSet* touches, UIEvent* event);

	virtual void touchesEnded(NSSet* touches, UIEvent* event);

	virtual void touchesCancelled(NSSet* touches, UIEvent* event);

private:
	Ref< TouchDeviceiOS > m_deviceTouch;
	Ref< GamepadDeviceiOS > m_deviceGamepad;
	Ref< MouseDeviceiOS > m_deviceMouse;
};

bool InputDriveriOSImpl::create(void* nativeWindowHandle)
{
	UIView* view = (UIView*)nativeWindowHandle;
	CGRect frame = [view frame];

	m_deviceTouch = new TouchDeviceiOS();
	if (!m_deviceTouch->create(nativeWindowHandle))
		return false;

	m_deviceGamepad = new GamepadDeviceiOS();
	if (!m_deviceGamepad->create(nativeWindowHandle))
		return false;

	m_deviceMouse = new MouseDeviceiOS();
	if (!m_deviceMouse->create(nativeWindowHandle))
		return false;

	UITouchView* touchView = [[UITouchView alloc] initWithFrame: frame];
	[touchView setCallback: this];

	touchView.multipleTouchEnabled = YES;
	touchView.exclusiveTouch = YES;

	[view addSubview: touchView];
	return true;
}

int InputDriveriOSImpl::getDeviceCount()
{
	return 3;
}

Ref< IInputDevice > InputDriveriOSImpl::getDevice(int index)
{
	if (index == 0)
		return m_deviceTouch;
	else if (index == 1)
		return m_deviceGamepad;
	else if (index == 2)
		return m_deviceMouse;
	else
		return 0;
}

void InputDriveriOSImpl::touchesBegan(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesBegan(touches, event);
	m_deviceGamepad->touchesBegan(touches, event);
	m_deviceMouse->touchesBegan(touches, event);
}

void InputDriveriOSImpl::touchesMoved(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesMoved(touches, event);
	m_deviceGamepad->touchesMoved(touches, event);
	m_deviceMouse->touchesMoved(touches, event);
}

void InputDriveriOSImpl::touchesEnded(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesEnded(touches, event);
	m_deviceGamepad->touchesEnded(touches, event);
	m_deviceMouse->touchesEnded(touches, event);
}

void InputDriveriOSImpl::touchesCancelled(NSSet* touches, UIEvent* event)
{
	m_deviceTouch->touchesCancelled(touches, event);
	m_deviceGamepad->touchesCancelled(touches, event);
	m_deviceMouse->touchesCancelled(touches, event);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriveriOS", 0, InputDriveriOS, IInputDriver)

InputDriveriOS::InputDriveriOS()
{
}

bool InputDriveriOS::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	m_impl = new InputDriveriOSImpl();
	if (!m_impl->create(syswin.view))
	{
		m_impl = 0;
		return false;
	}
	return true;
}

int InputDriveriOS::getDeviceCount()
{
	return m_impl->getDeviceCount();
}

Ref< IInputDevice > InputDriveriOS::getDevice(int index)
{
	return m_impl->getDevice(index);
}

IInputDriver::UpdateResult InputDriveriOS::update()
{
	return UrOk;
}

	}
}
