#include <stddef.h>
#include <shader/wave_psslc.h>
#include "Render/Ps4/Editor/ProgramCompilerPs4.h"
#include "Render/Ps4/Editor/Pssl/Pssl.h"
#include "Render/Ps4/Editor/Pssl/PsslProgram.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs4", 0, ProgramCompilerPs4, IProgramCompiler)

ProgramCompilerPs4::ProgramCompilerPs4()
{
}

const wchar_t* ProgramCompilerPs4::getPlatformSignature() const
{
	return L"GNM";
}

Ref< ProgramResource > ProgramCompilerPs4::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	PsslProgram psslProgram;
	if (!Pssl().generate(shaderGraph, psslProgram))
		return 0;

	return 0;
}

bool ProgramCompilerPs4::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	PsslProgram psslProgram;
	if (!Pssl().generate(shaderGraph, psslProgram))
		return false;

	outVertexShader = psslProgram.getVertexShader();
	outPixelShader = psslProgram.getPixelShader();

	return true;
}

	}
}