#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/HlslContext.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

bool Hlsl::generate(
	const ShaderGraph* shaderGraph,
	HlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate HLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	HlslContext cx(shaderGraph);

	if (!cx.getEmitter().emit(cx, pixelOutputs[0]))
	{
		log::error << L"Unable to generate HLSL shader; emitter failed with pixel graph" << Endl;
		return false;
	}

	if (!cx.getEmitter().emit(cx, vertexOutputs[0]))
	{
		log::error << L"Unable to generate HLSL shader; emitter failed with vertex graph" << Endl;
		return false;
	}

	outProgram = HlslProgram(
		cx.getVertexShader().getGeneratedShader(cx.needVPos()),
		cx.getPixelShader().getGeneratedShader(cx.needVPos()),
		cx.getState()
	);

	return true;
}

	}
}
