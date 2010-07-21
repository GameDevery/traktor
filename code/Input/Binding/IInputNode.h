#ifndef traktor_input_IInputNode_H
#define traktor_input_IInputNode_H

#include "Core/Serialization/ISerializable.h"

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
	
class InputValueSet;

/*! \brief Abstract input signal node.
 * \ingroup Input
 *
 * Input node is used by the states in order
 * to evaluate source values into state ready
 * output value.
 */
class T_DLLCLASS IInputNode : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Instance : public IRefCount {};

	virtual Ref< Instance > createInstance() const = 0;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const = 0;
};

	}
}

#endif	// traktor_input_IInputNode_H
