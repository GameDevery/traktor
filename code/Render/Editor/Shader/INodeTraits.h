#ifndef traktor_render_INodeTraits_H
#define traktor_render_INodeTraits_H

#include "Core/Object.h"
#include "Render/Editor/Shader/Constant.h"
#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! \brief Shader graph node traits.
 * \ingroup Render
 *
 * Node traits define behavior of nodes
 * such as type conversions etc.
 *
 * These are used by optimizing steps in
 * order to have those steps independent
 * of node types.
 */
class INodeTraits : public Object
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getNodeTypes() const = 0;

	/*! \brief Determine type of output pin from given types of all input pins.
	 */
	virtual PinType getOutputPinType(
		const Node* node,
		const OutputPin* outputPin,
		const PinType* inputPinTypes
	) const = 0;
	
	/*! \brief Get type of input pin.
	 */
	virtual PinType getInputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin,
		const PinType* outputPinTypes
	) const = 0;

	/*! \brief Get input pin independence group.
	 *
	 * A independence group is defined as being a number
	 * in which input pins can be swapped without
	 * changing the result of the node.
	 * For example both input pins of an Add node
	 * can be swapped without changing the result.
	 */
	virtual int32_t getInputPinGroup(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin
	) const = 0;

	/*! \brief Evaluate output pin from a fully constant input set.
	 */
	virtual bool evaluateFull(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* outputPin,
		const Constant* inputConstants,
		Constant& outputConstant
	) const = 0;

	/*! \brief Evaluate output pin from partially constant input set.
	 */
	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const Constant* inputConstants,
		Constant& outputConstant
	) const = 0;

	/*! \brief Evaluate rewire output pin from partially constant input set.
	 */
	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const OutputPin** inputOutputPins,
		const Constant* inputConstants,
		const OutputPin*& foldOutputPin
	) const = 0;

	/*! \brief Get node traits.
	 */
	static const INodeTraits* find(const Node* node);
};

	}
}

#endif	// traktor_render_INodeTraits_H
