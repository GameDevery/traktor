#include <cstring>
#include <sstream>

#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/doc.h>
#include <SPIRV/disassemble.h>

#include <spirv-tools/optimizer.hpp>

#include <spirv_hlsl.hpp>
#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>

#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Render/Editor/Glsl/GlslContext.h"
#include "Render/Editor/Glsl/GlslImage.h"
#include "Render/Editor/Glsl/GlslSampler.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Editor/Glsl/GlslTexture.h"
#include "Render/Editor/Glsl/GlslUniformBuffer.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/Editor/ProgramCompilerVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

TBuiltInResource getDefaultBuiltInResource()
{
	TBuiltInResource bir = {};

	bir.maxLights = 32;
	bir.maxClipPlanes = 6;
	bir.maxTextureUnits = 32;
	bir.maxTextureCoords = 32;
	bir.maxVertexAttribs = 64;
	bir.maxVertexUniformComponents = 4096;
	bir.maxVaryingFloats = 64;
	bir.maxVertexTextureImageUnits = 32;
	bir.maxCombinedTextureImageUnits = 80;
	bir.maxTextureImageUnits = 32;
	bir.maxFragmentUniformComponents = 4096;
	bir.maxDrawBuffers = 32;
	bir.maxVertexUniformVectors = 128;
	bir.maxVaryingVectors = 8;
	bir.maxFragmentUniformVectors = 16;
	bir.maxVertexOutputVectors = 16;
	bir.maxFragmentInputVectors = 15;
	bir.minProgramTexelOffset = -8;
	bir.maxProgramTexelOffset = 7;
	bir.maxClipDistances = 8;
	bir.maxComputeWorkGroupCountX = 65535;
	bir.maxComputeWorkGroupCountY = 65535;
	bir.maxComputeWorkGroupCountZ = 65535;
	bir.maxComputeWorkGroupSizeX = 1024;
	bir.maxComputeWorkGroupSizeY = 1024;
	bir.maxComputeWorkGroupSizeZ = 64;
	bir.maxComputeUniformComponents = 1024;
	bir.maxComputeTextureImageUnits = 16;
	bir.maxComputeImageUniforms = 8;
	bir.maxComputeAtomicCounters = 8;
	bir.maxComputeAtomicCounterBuffers = 1;
	bir.maxVaryingComponents = 60;
	bir.maxVertexOutputComponents = 64;
	bir.maxGeometryInputComponents = 64;
	bir.maxGeometryOutputComponents = 128;
	bir.maxFragmentInputComponents = 128;
	bir.maxImageUnits = 8;
	bir.maxCombinedImageUnitsAndFragmentOutputs = 8;
	bir.maxCombinedShaderOutputResources = 8;
	bir.maxImageSamples = 0;
	bir.maxVertexImageUniforms = 0;
	bir.maxTessControlImageUniforms = 0;
	bir.maxTessEvaluationImageUniforms = 0;
	bir.maxGeometryImageUniforms = 0;
	bir.maxFragmentImageUniforms = 8;
	bir.maxCombinedImageUniforms = 8;
	bir.maxGeometryTextureImageUnits = 16;
	bir.maxGeometryOutputVertices = 256;
	bir.maxGeometryTotalOutputComponents = 1024;
	bir.maxGeometryUniformComponents = 1024;
	bir.maxGeometryVaryingComponents = 64;
	bir.maxTessControlInputComponents = 128;
	bir.maxTessControlOutputComponents = 128;
	bir.maxTessControlTextureImageUnits = 16;
	bir.maxTessControlUniformComponents = 1024;
	bir.maxTessControlTotalOutputComponents = 4096;
	bir.maxTessEvaluationInputComponents = 128;
	bir.maxTessEvaluationOutputComponents = 128;
	bir.maxTessEvaluationTextureImageUnits = 16;
	bir.maxTessEvaluationUniformComponents = 1024;
	bir.maxTessPatchComponents = 120;
	bir.maxPatchVertices = 32;
	bir.maxTessGenLevel = 64;
	bir.maxViewports = 16;
	bir.maxVertexAtomicCounters = 0;
	bir.maxTessControlAtomicCounters = 0;
	bir.maxTessEvaluationAtomicCounters = 0;
	bir.maxGeometryAtomicCounters = 0;
	bir.maxFragmentAtomicCounters = 8;
	bir.maxCombinedAtomicCounters = 8;
	bir.maxAtomicCounterBindings = 1;
	bir.maxVertexAtomicCounterBuffers = 0;
	bir.maxTessControlAtomicCounterBuffers = 0;
	bir.maxTessEvaluationAtomicCounterBuffers = 0;
	bir.maxGeometryAtomicCounterBuffers = 0;
	bir.maxFragmentAtomicCounterBuffers = 1;
	bir.maxCombinedAtomicCounterBuffers = 1;
	bir.maxAtomicCounterBufferSize = 16384;
	bir.maxTransformFeedbackBuffers = 4;
	bir.maxTransformFeedbackInterleavedComponents = 64;
	bir.maxCullDistances = 8;
	bir.maxCombinedClipAndCullDistances = 8;
	bir.maxSamples = 4;

	bir.maxMeshOutputVerticesNV = 0;
    bir.maxMeshOutputPrimitivesNV = 0;
    bir.maxMeshWorkGroupSizeX_NV = 0;
    bir.maxMeshWorkGroupSizeY_NV = 0;
    bir.maxMeshWorkGroupSizeZ_NV = 0;
    bir.maxTaskWorkGroupSizeX_NV = 0;
    bir.maxTaskWorkGroupSizeY_NV = 0;
    bir.maxTaskWorkGroupSizeZ_NV = 0;
    bir.maxMeshViewCountNV = 0;
	bir.maxDualSourceDrawBuffersEXT = 0;

	bir.limits.nonInductiveForLoops = 1;
	bir.limits.whileLoops = 1;
	bir.limits.doWhileLoops = 1;
	bir.limits.generalUniformIndexing = 1;
	bir.limits.generalAttributeMatrixVectorIndexing = 1;
	bir.limits.generalVaryingIndexing = 1;
	bir.limits.generalSamplerIndexing = 1;
	bir.limits.generalVariableIndexing = 1;
	bir.limits.generalConstantMatrixVectorIndexing = 1;

	return bir;
}

const uint32_t c_parameterTypeWidths[] = { 1, 4, 16, 0, 0, 0 };

void performOptimization(bool convertRelaxedToHalf, AlignedVector< uint32_t >& spirv)
{
	spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;

	spvtools::Optimizer optimizer(target_env);
	optimizer.SetMessageConsumer([](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message) {
		switch (level)
		{
		case SPV_MSG_FATAL:
		case SPV_MSG_INTERNAL_ERROR:
		case SPV_MSG_ERROR:
			log::error << L"SPIRV optimization error: ";
			if (source)
				log::error << mbstows(source) << L":";
			log::error << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::error << L" " << mbstows(message);
			log::error << Endl;
			break;

		case SPV_MSG_WARNING:
			log::warning << L"SPIRV optimization warning: ";
			if (source)
				log::warning << mbstows(source) << L":";
			log::warning << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::warning << L" " << mbstows(message);
			log::warning << Endl;
			break;

		case SPV_MSG_INFO:
		case SPV_MSG_DEBUG:
				log::info << L"SPIRV optimization info: ";
			if (source)
				log::info << mbstows(source) << L":";
			log::info << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::info << L" " << mbstows(message);
			log::info << Endl;
			break;

		default:
			break;
		}
	});

	optimizer.RegisterPerformancePasses();

	if (convertRelaxedToHalf)
		optimizer.RegisterPass(spvtools::CreateConvertRelaxedToHalfPass());

	spvtools::OptimizerOptions spvOptOptions;
	std::vector< uint32_t > opted;
	if (optimizer.Run(spirv.c_ptr(), spirv.size(), &opted, spvOptOptions))
		spirv = AlignedVector< uint32_t >(opted.begin(), opted.end());
	else
		log::warning << L"SPIR-V optimizer failed; using unoptimized IL." << Endl;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerVk", 0, ProgramCompilerVk, IProgramCompiler)

ProgramCompilerVk::ProgramCompilerVk()
{
	static bool s_initialized = false;
	if (!s_initialized)
	{
		glslang::InitializeProcess();
		s_initialized = true;
	}
}

const wchar_t* ProgramCompilerVk::getRendererSignature() const
{
	return L"Vulkan";
}

Ref< ProgramResource > ProgramCompilerVk::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	Stats* outStats
) const
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;
	RefArray< ComputeOutput > computeOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

	GlslContext cx(shaderGraph, settings, GlslDialect::Vulkan);

	glslang::TProgram* program = new glslang::TProgram();
	glslang::TShader* vertexShader = nullptr;
	glslang::TShader* fragmentShader = nullptr;
	glslang::TShader* computeShader = nullptr;

	if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
	{
		const auto defaultBuiltInResource = getDefaultBuiltInResource();

		cx.getEmitter().emit(cx, pixelOutputs[0]);
		cx.getEmitter().emit(cx, vertexOutputs[0]);

		GlslRequirements vertexRequirements = cx.requirements();
		GlslRequirements fragmentRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		const std::string vertexShaderText = wstombs(cx.getVertexShader().getGeneratedShader(settings, layout, vertexRequirements));
		const std::string fragmentShaderText = wstombs(cx.getFragmentShader().getGeneratedShader(settings, layout, fragmentRequirements));

		// Vertex shader.
		const char* vst = vertexShaderText.c_str();
		vertexShader = new glslang::TShader(EShLangVertex);
		vertexShader->setStrings(&vst, 1);
		vertexShader->setEntryPoint("main");
		bool vertexResult = vertexShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgSuppressWarnings));
		if (vertexShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(vertexShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}
#if defined(_DEBUG)
		if (vertexShader->getInfoDebugLog())
			log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;
#endif
		if (!vertexResult)
			return nullptr;

		// Fragment shader.
		const char* fst = fragmentShaderText.c_str();
		fragmentShader = new glslang::TShader(EShLangFragment);
		fragmentShader->setStrings(&fst, 1);
		fragmentShader->setEntryPoint("main");
		bool fragmentResult = fragmentShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgSuppressWarnings));
		if (fragmentShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(fragmentShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}
#if defined(_DEBUG)
		if (fragmentShader->getInfoDebugLog())
			log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;
#endif
		if (!fragmentResult)
			return nullptr;

		program->addShader(vertexShader);
		program->addShader(fragmentShader);
	}
	else if (computeOutputs.size() == 1)
	{
		const auto defaultBuiltInResource = getDefaultBuiltInResource();

		cx.getEmitter().emit(cx, computeOutputs[0]);

		GlslRequirements computeRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		const char* computeShaderText = strdup(wstombs(cx.getComputeShader().getGeneratedShader(settings, layout, computeRequirements)).c_str());

		// Compute shader.
		computeShader = new glslang::TShader(EShLangCompute);
		computeShader->setStrings(&computeShaderText, 1);
		computeShader->setEntryPoint("main");
		bool computeResult = computeShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgSuppressWarnings));
		if (computeShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(computeShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}
#if defined(_DEBUG)
		if (computeShader->getInfoDebugLog())
			log::info << mbstows(computeShader->getInfoDebugLog()) << Endl;
#endif
		if (!computeResult)
			return nullptr;

		program->addShader(computeShader);
	}
	else
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs." << Endl;
		return nullptr;
	}

	// Link program shaders.
	if (!program->link(EShMsgDefault))
		return nullptr;

	const int32_t optimize = (settings != nullptr ? settings->getProperty< int32_t >(L"Glsl.Vulkan.Optimize", 1) : 1);
	const bool convertRelaxedToHalf = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.ConvertRelaxedToHalf", false) : false);

	// Create output resource.
	Ref< ProgramResourceVk > programResource = new ProgramResourceVk();
	programResource->m_renderState = cx.getRenderState();

	// Generate SPIR-V from program AST.
	auto vsi = program->getIntermediate(EShLangVertex);
	if (vsi != nullptr)
	{
		std::vector< uint32_t > vs;
		glslang::GlslangToSpv(*vsi, vs);
		programResource->m_vertexShader = AlignedVector< uint32_t >(vs.begin(), vs.end());
		if (optimize > 0)
			performOptimization(convertRelaxedToHalf, programResource->m_vertexShader);
	}

	auto fsi = program->getIntermediate(EShLangFragment);
	if (fsi != nullptr)
	{
		std::vector< uint32_t > fs;
		glslang::GlslangToSpv(*fsi, fs);
		programResource->m_fragmentShader = AlignedVector< uint32_t >(fs.begin(), fs.end());
		if (optimize > 0)
			performOptimization(convertRelaxedToHalf, programResource->m_fragmentShader);
	}

	auto csi = program->getIntermediate(EShLangCompute);
	if (csi != nullptr)
	{
		std::vector< uint32_t > cs;
		glslang::GlslangToSpv(*csi, cs);
		programResource->m_computeShader = AlignedVector< uint32_t >(cs.begin(), cs.end());
		if (optimize > 0)
			performOptimization(convertRelaxedToHalf, programResource->m_computeShader);
	}

	// Map parameters to uniforms.
	struct ParameterMapping
	{
		uint32_t buffer;
		uint32_t offset;
		uint32_t length;
	};
	std::map< std::wstring, ParameterMapping > parameterMapping;

	for (auto resource : cx.getLayout().get())
	{
		if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			programResource->m_samplers.push_back(ProgramResourceVk::SamplerDesc(
				sampler->getBinding(GlslDialect::Vulkan),
				sampler->getStages(),
				sampler->getState()
			));
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			auto& pm = parameterMapping[texture->getName()];
			pm.buffer = texture->getBinding(GlslDialect::Vulkan);
			pm.offset = (uint32_t)programResource->m_textures.size();
			pm.length = 0;

			programResource->m_textures.push_back(ProgramResourceVk::TextureDesc(
				texture->getName(),
				texture->getBinding(GlslDialect::Vulkan),
				texture->getStages()
			));
		}
		else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		{
			auto& pm = parameterMapping[image->getName()];
			pm.buffer = image->getBinding(GlslDialect::Vulkan);
			pm.offset = (uint32_t)programResource->m_textures.size();
			pm.length = 0;

			programResource->m_textures.push_back(ProgramResourceVk::TextureDesc(
				image->getName(),
				image->getBinding(GlslDialect::Vulkan),
				image->getStages()
			));
		}
		else if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
		{
			uint32_t size = 0;
			for (auto uniform : uniformBuffer->get())
			{
				if (uniform.length > 1)
					size = alignUp(size, 4);

				auto& pm = parameterMapping[uniform.name];
				pm.buffer = uniformBuffer->getBinding(GlslDialect::Vulkan);
				pm.offset = size;
				pm.length = glsl_type_width(uniform.type) * uniform.length;

				size += glsl_type_width(uniform.type) * uniform.length;
			}
			programResource->m_uniformBufferSizes[uniformBuffer->getBinding(GlslDialect::Vulkan)] = size;
		}
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			auto& pm = parameterMapping[storageBuffer->getName()];
			pm.buffer = storageBuffer->getBinding(GlslDialect::Vulkan);
			pm.offset = (uint32_t)programResource->m_sbuffers.size();
			pm.length = 0;

			programResource->m_sbuffers.push_back(ProgramResourceVk::SBufferDesc(
				storageBuffer->getName(),
				storageBuffer->getBinding(GlslDialect::Vulkan),
				storageBuffer->getStages()
			));
		}
	}

	for (auto p : cx.getParameters())
	{
		if (p.type <= PtMatrix)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
		else if (p.type >= PtTexture2D && p.type <= PtTextureCube)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
		else if (p.type >= PtStructBuffer)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
	}

	// Calculate hashes.
	{
		Adler32 checksum;
		checksum.begin();
		checksum.feed(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size() * sizeof(uint32_t));
		checksum.end();
		programResource->m_vertexShaderHash = checksum.get();
	}
	{
		Adler32 checksum;
		checksum.begin();
		checksum.feed(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size() * sizeof(uint32_t));
		checksum.end();
		programResource->m_fragmentShaderHash = checksum.get();
	}
	{
		Adler32 checksum;
		checksum.begin();
		checksum.feed(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size() * sizeof(uint32_t));
		checksum.end();
		programResource->m_computeShaderHash = checksum.get();
	}
	{
		Adler32 checksum;
		checksum.begin();
		checksum.feed(cx.getRenderState());
		checksum.feed(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size() * sizeof(uint32_t));
		checksum.feed(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size() * sizeof(uint32_t));
		checksum.feed(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size() * sizeof(uint32_t));
		checksum.end();
		programResource->m_shaderHash = checksum.get();
	}
	{
		Adler32 checksum;
		checksum.begin();

		for (int32_t i = 0; i < 3; ++i)
		{
			if (programResource->m_uniformBufferSizes[i] > 0)
			{
				checksum.feed(L"UB");
				checksum.feed(i);
			}
		}

		checksum.feed(programResource->m_samplers.size());
		for (uint32_t i = 0; i < programResource->m_samplers.size(); ++i)
		{
			checksum.feed(L"S");
			checksum.feed(i);
			checksum.feed(programResource->m_samplers[i].binding);
			checksum.feed(programResource->m_samplers[i].stages);
		}

		checksum.feed(programResource->m_textures.size());
		for (uint32_t i = 0; i < programResource->m_textures.size(); ++i)
		{
			checksum.feed(L"T");
			checksum.feed(i);
			checksum.feed(programResource->m_textures[i].binding);
			checksum.feed(programResource->m_textures[i].stages);
		}

		checksum.feed(programResource->m_sbuffers.size());
		for (uint32_t i = 0; i < programResource->m_sbuffers.size(); ++i)
		{
			checksum.feed(L"SB");
			checksum.feed(i);
			checksum.feed(programResource->m_sbuffers[i].binding);
			checksum.feed(programResource->m_sbuffers[i].stages);
		}

		checksum.end();
		programResource->m_layoutHash = checksum.get();
	}

	log::debug << L"Vulkan program \"" << name << L"\" compiled successfully:" << Endl;
	log::debug << IncreaseIndent;
	log::debug << L"m_vertexShaderHash = " << str(L"0x%08x", programResource->m_vertexShaderHash) << Endl;
	log::debug << L"m_fragmentShaderHash = " << str(L"0x%08x", programResource->m_fragmentShaderHash) << Endl;
	log::debug << L"m_computeShaderHash = " << str(L"0x%08x", programResource->m_computeShaderHash) << Endl;
	log::debug << L"m_shaderHash = " << str(L"0x%08x", programResource->m_shaderHash) << Endl;
	log::debug << L"m_layoutHash = " << str(L"0x%08x", programResource->m_layoutHash) << Endl;
	log::debug << DecreaseIndent;

	// \note Need to delete program before shaders due to glslang weirdness.
	delete program;
	delete fragmentShader;
	delete vertexShader;
	delete computeShader;
	return programResource;
}

bool ProgramCompilerVk::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	std::wstring crossDialect = settings->getProperty< std::wstring >(L"Glsl.Vulkan.CrossDialect");

	// No dialect means we should output our generated GLSL.
	if (crossDialect.empty())
	{
		RefArray< VertexOutput > vertexOutputs;
		RefArray< PixelOutput > pixelOutputs;
		RefArray< ComputeOutput > computeOutputs;

		shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
		shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
		shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

		GlslContext cx(shaderGraph, settings, GlslDialect::Vulkan);

		if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
		{
			bool result = true;
			result &= cx.getEmitter().emit(cx, pixelOutputs[0]);
			result &= cx.getEmitter().emit(cx, vertexOutputs[0]);
			if (!result)
			{
				log::error << L"Unable to generate Vulkan GLSL shader; GLSL emitter failed." << Endl;
				return false;
			}
		}
		else if (computeOutputs.size() == 1)
		{
			bool result = cx.getEmitter().emit(cx, computeOutputs[0]);
			if (!result)
			{
				log::error << L"Unable to generate Vulkan GLSL shader; GLSL emitter failed." << Endl;
				return false;
			}
		}
		else
		{
			log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs." << Endl;
			return false;
		}

		const auto& layout = cx.getLayout();
		StringOutputStream ss;

#if 0
		ss << L"// Layout" << Endl;
		for (auto resource : layout.get())
		{
			if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
			{
				ss << L"// [" << sampler->getBinding() << L"] = sampler" << Endl;
				ss << L"//   .name = \"" << sampler->getName() << L"\"" << Endl;
			}
			else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
			{
				ss << L"// [" << texture->getBinding() << L"] = texture" << Endl;
				ss << L"//   .name = \"" << texture->getName() << L"\"" << Endl;
				ss << L"//   .type = " << int32_t(texture->getUniformType()) << Endl;
			}
			else if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
			{
				ss << L"// [" << uniformBuffer->getBinding() << L"] = uniform buffer" << Endl;
				ss << L"//   .name = \"" << uniformBuffer->getName() << L"\"" << Endl;
				ss << L"//   .uniforms = {" << Endl;
				for (auto uniform : uniformBuffer->get())
				{
					ss << L"//      " << int32_t(uniform.type) << L" \"" << uniform.name << L"\" " << uniform.length << Endl;
				}
				ss << L"//   }" << Endl;
			}
			else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
			{
				ss << L"// [" << image->getBinding() << L"] = image" << Endl;
				ss << L"//   .name = \"" << image->getName() << L"\"" << Endl;
			}
			else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
			{
				ss << L"// [" << storageBuffer->getBinding() << L"] = storage buffer" << Endl;
				ss << L"//   .name = \"" << storageBuffer->getName() << L"\"" << Endl;
				ss << L"//   .elements = {" << Endl;
				for (auto element : storageBuffer->get())
				{
					ss << L"//      " << int32_t(element.type) << L" \"" << element.name << Endl;
				}
				ss << L"//   }" << Endl;
			}
		}
#endif

		GlslRequirements requirements = cx.requirements();

		// Vertex
		{
			StringOutputStream vss;
			vss << cx.getVertexShader().getGeneratedShader(settings, layout, requirements);
			vss << Endl;
			vss << ss.str();
			vss << Endl;
			outVertexShader = vss.str();
		}

		// Pixel
		{
			StringOutputStream fss;
			fss << cx.getFragmentShader().getGeneratedShader(settings, layout, requirements);
			fss << Endl;
			fss << ss.str();
			fss << Endl;
			outPixelShader = fss.str();
		}

		// Compute
		{
			StringOutputStream css;
			css << cx.getComputeShader().getGeneratedShader(settings, layout, requirements);
			css << Endl;
			css << ss.str();
			css << Endl;
			outComputeShader = css.str();
		}
	}
	else
	{
		Ref< ProgramResourceVk > programResource = checked_type_cast< ProgramResourceVk* >(compile(
			shaderGraph,
			settings,
			name,
			nullptr
		));
		if (!programResource)
			return false;

		if (crossDialect == L"SPIRV")
		{
			if (!programResource->m_vertexShader.empty())
			{
				std::stringstream ss;
				std::vector< uint32_t > v(programResource->m_vertexShader.begin(), programResource->m_vertexShader.end());
				spv::Disassemble(ss, v);
				outVertexShader = mbstows(ss.str());
			}
			if (!programResource->m_fragmentShader.empty())
			{
				std::stringstream ss;
				std::vector< uint32_t > f(programResource->m_fragmentShader.begin(), programResource->m_fragmentShader.end());
				spv::Disassemble(ss, f);
				outPixelShader = mbstows(ss.str());
			}
			if (!programResource->m_computeShader.empty())
			{
				std::stringstream ss;
				std::vector< uint32_t > c(programResource->m_computeShader.begin(), programResource->m_computeShader.end());
				spv::Disassemble(ss, c);
				outComputeShader = mbstows(ss.str());
			}
			return true;
		}
		else if (crossDialect == L"MSL")
		{
			spirv_cross::CompilerMSL::Options options;
			options.platform = spirv_cross::CompilerMSL::Options::iOS;
			options.set_msl_version(2, 6);

			if (!programResource->m_vertexShader.empty())
			{
				spirv_cross::CompilerMSL msl(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size());
				msl.set_msl_options(options);
				msl.build_dummy_sampler_for_combined_images();
				msl.build_combined_image_samplers();
				std::string source = msl.compile();
				outVertexShader = mbstows(source);
			}

			if (!programResource->m_fragmentShader.empty())
			{
				spirv_cross::CompilerMSL msl(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size());
				msl.set_msl_options(options);
				msl.build_dummy_sampler_for_combined_images();
				msl.build_combined_image_samplers();
				std::string source = msl.compile();
				outPixelShader = mbstows(source);
			}

			if (!programResource->m_computeShader.empty())
			{
				spirv_cross::CompilerMSL msl(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size());
				msl.set_msl_options(options);
				msl.build_dummy_sampler_for_combined_images();
				msl.build_combined_image_samplers();
				std::string source = msl.compile();
				outComputeShader = mbstows(source);
			}
		}
		else if (crossDialect == L"GLSL")
		{
			spirv_cross::CompilerGLSL::Options options;
			options.vulkan_semantics = true;

			if (!programResource->m_vertexShader.empty())
			{
				spirv_cross::CompilerGLSL glsl(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size());
				glsl.set_common_options(options);
				// glsl.build_dummy_sampler_for_combined_images();
				// glsl.build_combined_image_samplers();
				std::string source = glsl.compile();
				outVertexShader = mbstows(source);
			}

			if (!programResource->m_fragmentShader.empty())
			{
				spirv_cross::CompilerGLSL glsl(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size());
				glsl.set_common_options(options);
				// glsl.build_dummy_sampler_for_combined_images();
				// glsl.build_combined_image_samplers();
				std::string source = glsl.compile();
				outPixelShader = mbstows(source);
			}

			if (!programResource->m_computeShader.empty())
			{
				spirv_cross::CompilerGLSL glsl(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size());
				glsl.set_common_options(options);
				// glsl.build_dummy_sampler_for_combined_images();
				// glsl.build_combined_image_samplers();
				std::string source = glsl.compile();
				outComputeShader = mbstows(source);
			}
		}
		else if (crossDialect == L"HLSL")
		{
			spirv_cross::CompilerHLSL::Options options;

			if (!programResource->m_vertexShader.empty())
			{
				spirv_cross::CompilerHLSL hlsl(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size());
				hlsl.set_hlsl_options(options);
				hlsl.build_dummy_sampler_for_combined_images();
				hlsl.build_combined_image_samplers();
				std::string source = hlsl.compile();
				outVertexShader = mbstows(source);
			}

			if (!programResource->m_fragmentShader.empty())
			{
				spirv_cross::CompilerHLSL hlsl(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size());
				hlsl.set_hlsl_options(options);
				hlsl.build_dummy_sampler_for_combined_images();
				hlsl.build_combined_image_samplers();
				std::string source = hlsl.compile();
				outPixelShader = mbstows(source);
			}

			if (!programResource->m_computeShader.empty())
			{
				spirv_cross::CompilerHLSL hlsl(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size());
				hlsl.set_hlsl_options(options);
				hlsl.build_dummy_sampler_for_combined_images();
				hlsl.build_combined_image_samplers();
				std::string source = hlsl.compile();
				outComputeShader = mbstows(source);
			}
		}
		else
		{
			log::error << L"Unknown cross compile dialect \"" << crossDialect << L"\"." << Endl;
			return false;
		}
	}

	return true;
}

	}
}
