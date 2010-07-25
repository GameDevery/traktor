#ifndef traktor_input_InputSourceFabricator_H
#define traktor_input_InputSourceFabricator_H

#include <list>
#include <map>
#include "Core/Object.h"
#include "Input/InputTypes.h"

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

class CombinedInputSourceData;
class IInputDevice;
class IInputSourceData;
class InputSystem;

class T_DLLCLASS InputSourceFabricator : public Object
{
	T_RTTI_CLASS;

public:
	InputSourceFabricator(InputSystem* inputSystem, InputCategory category, bool analogue);
	
	Ref< IInputSourceData > update();

private:
	struct DeviceState
	{
		Ref< IInputDevice > device;
		std::map< InputDefaultControlType, float > values;
	};

	InputCategory m_category;
	bool m_analogue;
	Ref< CombinedInputSourceData > m_combinedData;
	Ref< IInputSourceData > m_outputData;
	std::list< DeviceState > m_deviceStates;
};
	
	}
}

#endif	// traktor_input_InputSourceFabricator_H
