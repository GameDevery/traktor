#pragma once

#include "Core/Platform.h"
#include "Input/IInputDevice.h"

struct AInputEvent;

namespace traktor
{
	namespace input
	{

class MouseDeviceAndroid : public IInputDevice
{
	T_RTTI_CLASS;

public:
	MouseDeviceAndroid(const SystemWindow& syswin);

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

private:
	friend class InputDriverAndroid;

	SystemWindow m_syswim;
	float m_axisX;
	float m_axisY;
	float m_positionX;
	float m_positionY;
	bool m_button;

	void handleInput(AInputEvent* event);
};

	}
}

