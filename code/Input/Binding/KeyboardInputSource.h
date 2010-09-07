#ifndef traktor_input_KeyboardInputSource_H
#define traktor_input_KeyboardInputSource_H

#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

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

class DeviceControl;
class DeviceControlManager;
class IInputDevice;
class InputSystem;

class T_DLLCLASS KeyboardInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	KeyboardInputSource(
		const std::vector< InputDefaultControlType >& controlTypes,
		DeviceControlManager* deviceControlManager
	);
	
	virtual ~KeyboardInputSource();
	
	virtual std::wstring getDescription() const;
	
	virtual void prepare(float T, float dT);

	virtual float read(float T, float dT);
	
private:
	struct Key
	{
		InputDefaultControlType controlType;
		RefArray< DeviceControl > deviceControls;
		bool state;
		
		bool operator == (const Key& rh) const;
	};
	
	Ref< DeviceControlManager > m_deviceControlManager;
	int32_t m_deviceCount;
	std::vector< Key > m_keys;
	bool m_state;
};

	}
}

#endif	// traktor_input_KeyboardInputSource_H
