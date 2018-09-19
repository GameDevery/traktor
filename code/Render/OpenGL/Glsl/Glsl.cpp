/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/OpenGL/Glsl/Glsl.h"
#include "Render/OpenGL/Glsl/GlslProgram.h"
#include "Render/OpenGL/Glsl/GlslContext.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

bool Glsl::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	GlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate GLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	GlslContext cx(shaderGraph, settings);
	if (!cx.emit(pixelOutputs[0]))
	{
		log::error << L"Unable to generate GLSL shader; emitter failed with fragment graph." << Endl;
		log::error << L"\t" << cx.getError() << Endl;
		return false;
	}
	if (!cx.emit(vertexOutputs[0]))
	{
		log::error << L"Unable to generate GLSL shader; emitter failed with vertex graph." << Endl;
		log::error << L"\t" << cx.getError() << Endl;
		return false;
	}

	GlslRequirements vertexRequirements = cx.requirements();
	vertexRequirements.derivatives = false;
	vertexRequirements.precisionHint = PhHigh;

	GlslRequirements fragmentRequirements = cx.requirements();

	outProgram = GlslProgram(
		cx.getVertexShader().getGeneratedShader(settings, name, vertexRequirements),
		cx.getFragmentShader().getGeneratedShader(settings, name, fragmentRequirements),
		cx.getTextures(),
		cx.getUniforms(),
		cx.getSamplers(),
		cx.getRenderState()
	);

	return true;
}

	}
}
