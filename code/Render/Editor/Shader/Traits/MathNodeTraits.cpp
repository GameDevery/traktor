#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MathNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MathNodeTraits", 0, MathNodeTraits, INodeTraits)

TypeInfoSet MathNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Abs >());
	typeSet.insert(&type_of< Add >());
	typeSet.insert(&type_of< ArcusCos >());
	typeSet.insert(&type_of< Clamp >());
	typeSet.insert(&type_of< Cos >());
	typeSet.insert(&type_of< Cross >());
	typeSet.insert(&type_of< Derivative >());
	typeSet.insert(&type_of< Div >());
	typeSet.insert(&type_of< Exp >());
	typeSet.insert(&type_of< Fraction >());
	typeSet.insert(&type_of< Interpolator >());
	typeSet.insert(&type_of< Log >());
	typeSet.insert(&type_of< Max >());
	typeSet.insert(&type_of< Min >());
	typeSet.insert(&type_of< Mul >());
	typeSet.insert(&type_of< MulAdd >());
	typeSet.insert(&type_of< Neg >());
	typeSet.insert(&type_of< Normalize >());
	typeSet.insert(&type_of< Pow >());
	typeSet.insert(&type_of< RecipSqrt >());
	typeSet.insert(&type_of< Reflect >());
	typeSet.insert(&type_of< Sign >());
	typeSet.insert(&type_of< Sin >());
	typeSet.insert(&type_of< Sqrt >());
	typeSet.insert(&type_of< Sub >());
	typeSet.insert(&type_of< Tan >());
	typeSet.insert(&type_of< Truncate >());
	return typeSet;
}

PinType MathNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputPinType = PntVoid;
	if (is_a< ArcusCos >(node) || is_a< Cos >(node) || is_a< Sin >(node) || is_a< Tan >(node))
		outputPinType = PntScalar1;
	else
	{
		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
			outputPinType = std::max< PinType >(
				outputPinType,
				inputPinTypes[i]
			);
	}
	return outputPinType;
}

PinType MathNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< ArcusCos >(node) || is_a< Cos >(node) || is_a< Sin >(node) || is_a< Tan >(node))
		return PntScalar1;
	else
		return outputPinTypes[0];
}

int32_t MathNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	if (
		is_a< Add >(node) ||
		is_a< Mul >(node) ||
		is_a< Max >(node) ||
		is_a< Min >(node)
	)
		return 0;
	else if (is_a< MulAdd >(node))
	{
		int32_t index = getInputPinIndex(node, inputPin);
		return (index == 0 || index == 1) ? 0 : 1;
	}
	else
		return getInputPinIndex(node, inputPin);
}

bool MathNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (is_a< Abs >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::abs(inputConstants[0][i]);
	}
	else if (is_a< Add >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] + inputConstants[1][i];
	}
	else if (is_a< ArcusCos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::acos(inputConstants[0][i]);
	}
	else if (const Clamp* clmp = dynamic_type_cast< const Clamp* >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = clamp(inputConstants[0][i], clmp->getMin(), clmp->getMax());
	}
	else if (is_a< Cos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::cos(inputConstants[0][i]);
	}
	//else if (is_a< Cross >(node))
	else if (is_a< Derivative >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = 0.0f;
	}
	else if (is_a< Div >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] / inputConstants[1][i];
	}
	//else if (is_a< Exp >(node))
	//else if (is_a< Fraction >(node))
	else if (is_a< Interpolator >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i];
	}
	//else if (is_a< Log >(node))
	else if (is_a< Max >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::max(inputConstants[0][i], inputConstants[1][i]);
	}
	else if (is_a< Min >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::min(inputConstants[0][i], inputConstants[1][i]);
	}
	else if (is_a< Mul >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] * inputConstants[1][i];
	}
	else if (is_a< MulAdd >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] * inputConstants[1][i] + inputConstants[2][i];
	}
	else if (is_a< Neg >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = -inputConstants[0][i];
	}
	else if (is_a< Normalize >(node))
	{
		float ln = std::sqrt(
			inputConstants[0][0] * inputConstants[0][0] +
			inputConstants[0][1] * inputConstants[0][1] +
			inputConstants[0][2] * inputConstants[0][2] +
			inputConstants[0][3] * inputConstants[0][3]
		);
		if (ln != 0.0f)
		{
			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				outputConstant[i] = inputConstants[0][i] / ln;
		}
		else
		{
			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				outputConstant[i] = 0.0f;
		}
	}
	else if (is_a< Pow >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::pow(inputConstants[0][i], inputConstants[1][i]);
	}
	else if (is_a< RecipSqrt >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = 1.0f / std::sqrt(inputConstants[0][i]);
	}
	//else if (is_a< Reflect >(node))
	else if (is_a< Sign >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = (inputConstants[0][i] >= 0.0f) ? 1.0f : -1.0f;
	}
	else if (is_a< Sin >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::sin(inputConstants[0][i]);
	}
	else if (is_a< Sqrt >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::sqrt(inputConstants[0][i]);
	}
	else if (is_a< Sub >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] - inputConstants[1][i];
	}
	else if (is_a< Tan >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::tan(inputConstants[0][i]);
	}
	else if (is_a< Truncate >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::floor(inputConstants[0][i]);
	}
	else
		return false;

	return true;
}

bool MathNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (is_a< Div >(node))
	{
		if (inputConstants[0].isZero() || inputConstants[1].isZero())
		{
			outputConstant = Constant(0.0f);
			return true;
		}
	}
	else if (is_a< Mul >(node))
	{
		if (inputConstants[0].isZero() || inputConstants[1].isZero())
		{
			outputConstant = Constant(0.0f);
			return true;
		}
	}
	else if (is_a< MulAdd >(node))
	{
		if (inputConstants[2].isZero() && (inputConstants[0].isZero() || inputConstants[1].isZero()))
		{
			outputConstant = Constant(0.0f);
			return true;
		}
	}
	else if (is_a< Pow >(node))
	{
		if (inputConstants[0].isZero())
		{
			outputConstant = Constant(1.0f);
			return true;
		}
	}
	return false;
}

bool MathNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	if (is_a< Div >(node))
	{
		if (inputConstants[1].isOne())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Mul >(node))
	{
		if (inputConstants[0].isOne())
		{
			foldOutputPin = inputOutputPins[1];
			return true;
		}
		else if (inputConstants[1].isOne())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< MulAdd >(node))
	{
		if (inputConstants[0].isZero() || inputConstants[1].isZero())
		{
			foldOutputPin = inputOutputPins[2];
			return true;
		}
	}
	else if (is_a< Pow >(node))
	{
		if (inputConstants[0].isOne())
		{
			foldOutputPin = inputOutputPins[1];
			return true;
		}
	}
	return false;
}

PinOrderType MathNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	if (
		is_a< Abs >(node) ||
		is_a< ArcusCos >(node) ||
		is_a< Clamp >(node) ||
		is_a< Cos >(node) ||
		is_a< Cross >(node) ||
		is_a< Derivative >(node) ||
		is_a< Exp >(node) ||
		is_a< Fraction >(node) ||
		is_a< Log >(node) ||
		is_a< Max >(node) ||
		is_a< Min >(node) ||
		is_a< Normalize >(node) ||
		is_a< Pow >(node) ||
		is_a< RecipSqrt >(node) ||
		is_a< Reflect >(node) ||
		is_a< Sign >(node) ||
		is_a< Sin >(node) ||
		is_a< Sqrt >(node) ||
		is_a< Tan >(node) ||
		is_a< Truncate >(node)
	)
		return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
	else if (is_a< Add >(node))
		return pinOrderMax(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< Div >(node))
		return pinOrderAdd(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< Interpolator >(node))
		return inputPinOrders[0];
	else if (is_a< Mul >(node))
		return pinOrderAdd(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< MulAdd >(node))
		return pinOrderMax(pinOrderAdd(inputPinOrders[0], inputPinOrders[1]), inputPinOrders[2]);
	else if (is_a< Neg >(node))
		return inputPinOrders[0];
	else if (is_a< Sub >(node))
		return pinOrderMax(inputPinOrders[0], inputPinOrders[1]);
	else
		return PotConstant;
}

	}
}
