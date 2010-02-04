#ifndef traktor_render_NodeCategories_H
#define traktor_render_NodeCategories_H

#include "Render/Nodes.h"
#include "Render/External.h"

namespace traktor
{
	namespace render
	{

struct NodeCategory
{
	const TypeInfo& type;
	const wchar_t* category;
	const wchar_t* description;
};

const NodeCategory c_nodeCateogories[] =
{
	{ type_of< Abs >(), L"SHADERGRAPH_ARITHMETIC", L"Absolute value" },
	{ type_of< Add >(), L"SHADERGRAPH_ARITHMETIC", L"Addition" },
	{ type_of< ArcusCos >(), L"SHADERGRAPH_TRIGONOMETRY", L"Inverse cosine" },
	{ type_of< ArcusTan >(), L"SHADERGRAPH_TRIGONOMETRY", L"Inverse tangent" },
	{ type_of< Branch >(), L"SHADERGRAPH_PERMUTATION", L"Static branch permutation" },
	{ type_of< Clamp >(), L"SHADERGRAPH_CONDITIONAL", L"Clamp values within range" },
	{ type_of< Color >(), L"SHADERGRAPH_VALUE", L"Color constant" },
	{ type_of< Comment >(), L"SHADERGRAPH_MISCELLANEOUS", L"Comment" },
	{ type_of< Conditional >(), L"SHADERGRAPH_CONDITIONAL", L"Evaluate different branches based on condition" },
	{ type_of< Cos >(), L"SHADERGRAPH_TRIGONOMETRY", L"Cosine" },
	{ type_of< Cross >(), L"SHADERGRAPH_ALGEBRA", L"Cross product of two vectors" },
	{ type_of< Derivative >(), L"SHADERGRAPH_ARITHMETIC", L"Partial derivate of function f(x,y) in respect to x or y screen coordinate" },
	{ type_of< Div >(), L"SHADERGRAPH_ARITHMETIC", L"Division" },
	{ type_of< Dot >(), L"SHADERGRAPH_ALGEBRA", L"Dot product of two vectors" },
	{ type_of< Exp >(), L"SHADERGRAPH_ARITHMETIC", L"Base e exponential of specified value" },
	{ type_of< External >(), L"SHADERGRAPH_MISCELLANEOUS", L"External shader fragment" },
	{ type_of< Fraction >(), L"SHADERGRAPH_MISCELLANEOUS", L"Extract fractional part of floating point number" },
	{ type_of< FragmentPosition >(), L"SHADERGRAPH_VALUE", L"Fragment position" },
	{ type_of< IndexedUniform >(), L"SHADERGRAPH_VALUE", L"Indexed uniform" },
	{ type_of< InputPort >(), L"SHADERGRAPH_VALUE", L"Input connection port into shader fragment" },
	{ type_of< Interpolator >(), L"SHADERGRAPH_VALUE", L"Interpolator passing of value from vertex into pixel shader" },
	{ type_of< Iterate >(), L"SHADERGRAPH_MISCELLANEOUS", L"Iterate value" },
	{ type_of< Length >(), L"SHADERGRAPH_ALGEBRA", L"Length of vector" },
	{ type_of< Lerp >(), L"SHADERGRAPH_MISCELLANEOUS", L"Linear interpolate value" },
	{ type_of< Log >(), L"SHADERGRAPH_ARITHMETIC", L"Logarithm of a number" },
	{ type_of< Matrix >(), L"SHADERGRAPH_MISCELLANEOUS", L"Build orthogonal matrix from axises and translation" },
	{ type_of< Max >(), L"SHADERGRAPH_CONDITIONAL", L"Per component maximum value" },
	{ type_of< Min >(), L"SHADERGRAPH_CONDITIONAL", L"Per component minimum value" },
	{ type_of< MixIn >(), L"SHADERGRAPH_MISCELLANEOUS", L"Build a vector from scalars" },
	{ type_of< MixOut >(), L"SHADERGRAPH_MISCELLANEOUS", L"Access elements from a vector" },
	{ type_of< Mul >(), L"SHADERGRAPH_ARITHMETIC", L"Multiplication" },
	{ type_of< MulAdd >(), L"SHADERGRAPH_ARITHMETIC", L"Multiply-then-add" },
	{ type_of< Neg >(), L"SHADERGRAPH_ARITHMETIC", L"Negate number" },
	{ type_of< Normalize >(), L"SHADERGRAPH_ALGEBRA", L"Normalize vector" },
	{ type_of< OutputPort >(), L"SHADERGRAPH_VALUE", L"Output connection port from shader fragment" },
	{ type_of< Polynomial >(), L"SHADERGRAPH_ARITHMETIC", L"Evaluate a polynomial" },
	{ type_of< Pow >(), L"SHADERGRAPH_ARITHMETIC", L"Power of function" },
	{ type_of< PixelOutput >(), L"SHADERGRAPH_VALUE", L"Pixel shader output" },
	{ type_of< Platform >(), L"SHADERGRAPH_CONDITIONAL", L"Select path based on type of renderer" },
	{ type_of< Reflect >(), L"SHADERGRAPH_ALGEBRA", L"Calculate reflected vector from a input direction and an axis" },
	{ type_of< Sampler >(), L"SHADERGRAPH_VALUE", L"Texture sampler" },
	{ type_of< Scalar >(), L"SHADERGRAPH_VALUE", L"Scalar constant" },
	{ type_of< Sin >(), L"SHADERGRAPH_TRIGONOMETRY", L"Sine" },
	{ type_of< Sqrt >(), L"SHADERGRAPH_ARITHMETIC", L"Square-root" },
	{ type_of< Sub >(), L"SHADERGRAPH_ARITHMETIC", L"Subtract" },
	{ type_of< Sum >(), L"SHADERGRAPH_ARITHMETIC", L"Summarize by evaluation branch from iteration" },
	{ type_of< Switch >(), L"SHADERGRAPH_CONDITIONAL", L"Switch" },
	{ type_of< Swizzle >(), L"SHADERGRAPH_MISCELLANEOUS", L"Swizzle elements of a vector" },
	{ type_of< Tan >(), L"SHADERGRAPH_TRIGONOMETRY", L"Tangent" },
	{ type_of< Transform >(), L"SHADERGRAPH_ALGEBRA", L"Transform vector by a matrix" },
	{ type_of< Transpose >(), L"SHADERGRAPH_ALGEBRA", L"Transpose matrix, swapping rows and columns" },
	{ type_of< Uniform >(), L"SHADERGRAPH_VALUE", L"Uniform" },
	{ type_of< Vector >(), L"SHADERGRAPH_VALUE", L"Vector constant" },
	{ type_of< VertexInput >(), L"SHADERGRAPH_VALUE", L"Read value from vertex stream" },
	{ type_of< VertexOutput >(), L"SHADERGRAPH_VALUE", L"Output value from vertex shader" }
};

	}
}

#endif	// traktor_render_NodeCategories_H
