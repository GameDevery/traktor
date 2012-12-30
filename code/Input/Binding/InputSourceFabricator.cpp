#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/CombinedInputSourceData.h"
#include "Input/Binding/GenericInputSourceData.h"
#include "Input/Binding/InputSourceFabricator.h"
#include "Input/Binding/KeyboardInputSourceData.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const float c_valueThreshold = 0.5f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputSourceFabricator", InputSourceFabricator, Object)

InputSourceFabricator::InputSourceFabricator(InputSystem* inputSystem, InputCategory category, bool analogue)
:	m_category(category)
,	m_analogue(analogue)
{
	int32_t deviceCount = inputSystem->getDeviceCount(m_category, true);
	for (int32_t i = 0; i < deviceCount; ++i)
	{
		Ref< IInputDevice > device = inputSystem->getDevice(m_category, i, true);
		if (device)
		{
			DeviceState ds;
			ds.device = device;
			m_deviceStates.push_back(ds);
		}
	}
}

Ref< IInputSourceData > InputSourceFabricator::update()
{
	// Have we already finished fabricating?
	if (m_outputData)
		return m_outputData;
		
	// Enumerate all devices and controls to see if anything has changed.
	for (std::list< DeviceState >::iterator i = m_deviceStates.begin(); i != m_deviceStates.end(); ++i)
	{
		for (int32_t j = DtUp; j <= DtKeyLastIndex; ++j)
		{
			InputDefaultControlType controlType = (InputDefaultControlType)j;
		
			int32_t control;
			if (!i->device->getDefaultControl(controlType, m_analogue, control))
				continue;
			
			float value = i->device->getControlValue(control);
			bool first = i->values.find(controlType) == i->values.end();

			// Wait until digital control has been released before we
			// start tracking it.
			if (
				!m_analogue &&
				asBoolean(value) &&
				first
			)
				continue;

			float dV = value - i->values[controlType];
			if (std::abs(dV) > c_valueThreshold)
			{
				if (!first)
				{
					// Control has been modified; record it's use.
					if (m_analogue)
					{
						// Analogue control cannot be combined, thus create generic
						// source and finish.
						
						bool inverted = false;
						if (m_category == CtMouse)
						{
							if (dV < 0.0f)
								inverted = true;
						}

						m_outputData = new GenericInputSourceData(
							m_category,
							std::distance(m_deviceStates.begin(), i),
							controlType,
							m_analogue,
							inverted
						);

						break;
					}
					else if (m_category == CtKeyboard)
					{
						if (asBoolean(value))
						{
							if (!m_keyboardData)
								m_keyboardData = new KeyboardInputSourceData();

							m_keyboardData->addControlType(controlType);
						}
						else
						{
							m_outputData = m_keyboardData;
						}
					}
					else
					{
						// Digital controls are chained until any control are
						// released and then we're finished.
						if (asBoolean(value))
						{
							if (!m_combinedData)
								m_combinedData = new CombinedInputSourceData(CombinedInputSource::CmAll);
	
							m_combinedData->addSource(new GenericInputSourceData(
								m_category,
								std::distance(m_deviceStates.begin(), i),
								controlType,
								m_analogue,
								false
							));
						}
						else
						{
							// Remove combined wrapper if it only contain one source.
							const RefArray< IInputSourceData >& sources = m_combinedData->getSources();
							if (sources.size() > 1)
								m_outputData = m_combinedData;
							else if (sources.size() == 1)
								m_outputData = sources[0];
	
							if (m_outputData)
								break;
						}
					}
				}
				i->values[controlType] = value;
			}
		}
		if (m_outputData)
			break;
	}
	
	return m_outputData;
}

	}
}
