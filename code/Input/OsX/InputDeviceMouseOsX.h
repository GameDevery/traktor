#pragma once

#import <Cocoa/Cocoa.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class InputDeviceMouseOsX : public IInputDevice
{
	T_RTTI_CLASS;

public:
	InputDeviceMouseOsX();

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

	void consumeEvent(NSEvent* event);

private:
	bool m_button[4];
	int32_t m_axis[5];
	uint64_t m_timeStamps[3];
	bool m_associated;
	bool m_exclusive;
	bool m_lastMouseValid;
	float m_scrollAccum;
};

	}
}

