#ifndef traktor_amalgam_IInputServer_H
#define traktor_amalgam_IInputServer_H

#include "Input/InputTypes.h"
#include "Amalgam/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputMapping;
class InputMappingStateData;
class InputSystem;
class RumbleEffectPlayer;

	}

	namespace amalgam
	{

/*! \brief Input server.
 *
 * "Input.Rumble"	- Rumble enable.
 */
class T_DLLCLASS IInputServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual bool createInputMapping(const input::InputMappingStateData* stateData) = 0;

	virtual bool fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue) = 0;

	virtual bool isFabricating() const = 0;

	virtual bool abortedFabricating() const = 0;

	virtual bool resetInputSource(const std::wstring& sourceId) = 0;

	virtual input::InputSystem* getInputSystem() = 0;

	virtual input::InputMapping* getInputMapping() = 0;

	virtual input::RumbleEffectPlayer* getRumbleEffectPlayer() = 0;
};

	}
}

#endif	// traktor_amalgam_IInputServer_H
