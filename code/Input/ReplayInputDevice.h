#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Input/IInputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class RecordInputScript;

/*! \brief Replay input device.
 * \ingroup Input
 */
class T_DLLCLASS ReplayInputDevice : public IInputDevice
{
	T_RTTI_CLASS;

public:
	ReplayInputDevice(IInputDevice* inputDevice, RecordInputScript* inputScript, bool loop);

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

private:
	Ref< IInputDevice > m_inputDevice;
	Ref< RecordInputScript > m_inputScript;
	bool m_loop;
	uint32_t m_frame;
};

	}
}

