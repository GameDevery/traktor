#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Ps4/Editor/Pssl/Pssl.h"
#include "Render/Ps4/Editor/Pssl/PsslContext.h"
#include "Render/Ps4/Editor/Pssl/PsslProgram.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

bool Pssl::generate(
	const ShaderGraph* shaderGraph,
	PsslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate PSSL shader; incorrect number of outputs (VS " << vertexOutputs.size() << L", PS " << pixelOutputs.size() << L")" << Endl;
		return false;
	}

	PsslContext cx(shaderGraph);

	if (!cx.getEmitter().emit(cx, pixelOutputs[0]))
	{
		log::error << L"Unable to generate PSSL shader; emitter failed with pixel graph" << Endl;
		return false;
	}

	if (!cx.getEmitter().emit(cx, vertexOutputs[0]))
	{
		log::error << L"Unable to generate PSSL shader; emitter failed with vertex graph" << Endl;
		return false;
	}

	outProgram = PsslProgram(
		cx.getVertexShader().getGeneratedShader(),
		cx.getPixelShader().getGeneratedShader()
	);

	return true;
}

	}
}