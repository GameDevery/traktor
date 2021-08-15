#pragma once

#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! Immutable graph node.
 * \ingroup Render
 *
 * Immutable graph nodes are nodes which
 * doesn't change their appearance, i.e. has a fixed
 * set of input and output pins.
 */
class T_DLLCLASS ImmutableNode : public Node
{
	T_RTTI_CLASS;

public:
	struct InputPinDesc
	{
		const wchar_t* name;
		bool optional;
	};

	struct OutputPinDesc
	{
		const wchar_t* name;
	};

	explicit ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins);

	virtual ~ImmutableNode();

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

private:
	InputPin* m_inputPins = nullptr;
	OutputPin* m_outputPins = nullptr;
	int32_t m_inputPinCount = 0;
	int32_t m_outputPinCount = 0;

	ImmutableNode& operator = (const ImmutableNode&) { T_FATAL_ERROR; return *this; }
};

	}
}

