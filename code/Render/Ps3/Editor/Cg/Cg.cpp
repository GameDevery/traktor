#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Ps3/Editor/Cg/Cg.h"
#include "Render/Ps3/Editor/Cg/CgProgram.h"
#include "Render/Ps3/Editor/Cg/CgContext.h"

namespace traktor
{
	namespace render
	{

bool Cg::generate(
	const ShaderGraph* shaderGraph,
	CgProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate CG shader; incorrect number of outputs" << Endl;
		return false;
	}

	CgContext cx(shaderGraph);

	if (!cx.getEmitter().emit(cx, pixelOutputs[0]))
	{
		log::error << L"Unable to generate CG shader; emitter failed with pixel graph" << Endl;
		return false;
	}

	if (!cx.getEmitter().emit(cx, vertexOutputs[0]))
	{
		log::error << L"Unable to generate CG shader; emitter failed with vertex graph" << Endl;
		return false;
	}

	outProgram = CgProgram(
		cx.getVertexShader().getGeneratedShader(cx.needVPos()),
		cx.getPixelShader().getGeneratedShader(cx.needVPos()),
		cx.getVertexShader().getSamplerTextures(),
		cx.getPixelShader().getSamplerTextures(),
		cx.getRenderState(),
		cx.getRegisterCount()
	);

	return true;
}

	}
}