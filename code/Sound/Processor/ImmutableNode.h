#pragma once

#include "Core/RefArray.h"
#include "Sound/Processor/InputPin.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"
#include "Sound/Processor/ProcessorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! Immutable processor graph node.
 * \ingroup Sound
 */
class T_DLLCLASS ImmutableNode : public Node
{
	T_RTTI_CLASS;

public:
	struct InputPinDesc
	{
		const wchar_t* name;
		NodePinType type;
		bool optional;
	};

	struct OutputPinDesc
	{
		const wchar_t* name;
		NodePinType type;
	};

	ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins);

	virtual size_t getInputPinCount() const override final;

	virtual const InputPin* getInputPin(size_t index) const override final;

	virtual size_t getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(size_t index) const override final;

private:
	RefArray< InputPin > m_inputPins;
	RefArray< OutputPin > m_outputPins;

	ImmutableNode& operator = (const ImmutableNode&) { T_FATAL_ERROR; return *this; }
};

	}
}
