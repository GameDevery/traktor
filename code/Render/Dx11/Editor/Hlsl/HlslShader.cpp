#include <cassert>
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Editor/Hlsl/HlslShader.h"
#include "Render/Editor/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{

HlslShader::HlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_interpolatorCount(0)
,	m_booleanRegisterCount(0)
,	m_nextTemporaryVariable(0)
,	m_needVPos(false)
,	m_needVFace(false)
,	m_needTargetSize(false)
,	m_needInstanceID(false)
{
	m_variableScopes.push_back(0);
	for (int32_t i = 0; i < BtLast; ++i)
		pushOutputStream((BlockType)i, new StringOutputStream());
}

HlslShader::~HlslShader()
{
	for (int32_t i = 0; i < BtLast; ++i)
		popOutputStream((BlockType)i);
}

bool HlslShader::haveInput(const std::wstring& inputName) const
{
	return m_inputs.find(inputName) != m_inputs.end();
}

void HlslShader::addInput(const std::wstring& inputName)
{
	m_inputs.insert(inputName);
}

HlslVariable* HlslShader::createTemporaryVariable(const OutputPin* outputPin, HlslType type)
{
	StringOutputStream ss;
	ss << L"v" << m_nextTemporaryVariable++;
	return createVariable(outputPin, ss.str(), type);
}

HlslVariable* HlslShader::createVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type)
{
	for (uint32_t i = m_variableScopes.back(); i < m_variables.size(); ++i)
	{
		const auto& v = m_variables[i];
		T_FATAL_ASSERT (v.outputPin != outputPin);
	}

	auto& v = m_variables.push_back();
	v.outputPin = outputPin;
	v.variable = new HlslVariable(outputPin->getNode(), variableName, type);
	return v.variable;
}

HlslVariable* HlslShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type)
{
	auto& v = m_outerVariables.push_back();
	v.outputPin = outputPin;
	v.variable = new HlslVariable(outputPin->getNode(), variableName, type);
	return v.variable;
}

HlslVariable* HlslShader::getVariable(const OutputPin* outputPin) const
{
	for (auto it = m_variables.rbegin(); it != m_variables.rend(); ++it)
	{
		if (it->outputPin == outputPin)
			return it->variable;
	}
	for (auto it = m_outerVariables.begin(); it != m_outerVariables.end(); ++it)
	{
		if (it->outputPin == outputPin)
			return it->variable;
	}
	return nullptr;
}

void HlslShader::pushScope()
{
	m_variableScopes.push_back(m_variables.size());
}

void HlslShader::popScope()
{
	m_variables.resize(m_variableScopes.back());
	m_variableScopes.pop_back();
}

int32_t HlslShader::allocateInterpolator()
{
	return m_interpolatorCount++;
}

int32_t HlslShader::allocateBooleanRegister()
{
	return m_booleanRegisterCount++;
}

void HlslShader::allocateVPos()
{
	m_needVPos = true;
}

void HlslShader::allocateVFace()
{
	m_needVFace = true;
}

void HlslShader::allocateInstanceID()
{
	m_needInstanceID = true;
}

void HlslShader::allocateTargetSize()
{
	m_needTargetSize = true;
}

bool HlslShader::defineScript(const std::wstring& signature)
{
	auto i = m_scripts.find(signature);
	if (i != m_scripts.end())
		return false;

	m_scripts.insert(signature);
	return true;
}

void HlslShader::addSampler(const std::wstring& sampler, const D3D11_SAMPLER_DESC& dsd)
{
	m_samplers.insert(std::make_pair(sampler, dsd));
}

const std::map< std::wstring, D3D11_SAMPLER_DESC >& HlslShader::getSamplers() const
{
	return m_samplers;
}

void HlslShader::addUniform(const std::wstring& uniform)
{
	m_uniforms.insert(uniform);
}

const std::set< std::wstring >& HlslShader::getUniforms() const
{
	return m_uniforms;
}

void HlslShader::pushOutputStream(BlockType blockType, StringOutputStream* outputStream)
{
	m_outputStreams[int(blockType)].push_back(outputStream);
}

void HlslShader::popOutputStream(BlockType blockType)
{
	if (!m_outputStreams[int(blockType)].empty())
		m_outputStreams[int(blockType)].pop_back();
}

StringOutputStream& HlslShader::getOutputStream(BlockType blockType)
{
	T_ASSERT(!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

std::wstring HlslShader::getGeneratedShader()
{
	StringOutputStream ss;

	std::wstring cbufferOnceText = getOutputStream(BtCBufferOnce).str();
	if (!cbufferOnceText.empty())
	{
		ss << L"cbuffer cbOnce" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferOnceText;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring cbufferFrameText = getOutputStream(BtCBufferFrame).str();
	if (!cbufferFrameText.empty() || m_needTargetSize)
	{
		ss << L"cbuffer cbFrame" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferFrameText;

		if (m_needTargetSize)
			ss << L"float4 _dx11_targetSize;" << Endl;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring cbufferDrawText = getOutputStream(BtCBufferDraw).str();
	if (!cbufferDrawText.empty())
	{
		ss << L"cbuffer cbDraw" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferDrawText;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring structsText = getOutputStream(BtStructs).str();
	if (!structsText.empty())
	{
		ss << structsText;
		ss << Endl;
	}

	std::wstring texturesText = getOutputStream(BtTextures).str();
	if (!texturesText.empty())
	{
		ss << texturesText;
		ss << Endl;
	}

	std::wstring samplersText = getOutputStream(BtSamplers).str();
	if (!samplersText.empty())
	{
		ss << samplersText;
		ss << Endl;
	}

	std::wstring inputDataText = getOutputStream(BtInput).str();
	if (!inputDataText.empty())
	{
		ss << L"struct InputData" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;
		ss << inputDataText;
		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring outputDataText = getOutputStream(BtOutput).str();
	if (!outputDataText.empty())
	{
		ss << L"struct OutputData" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;
		ss << outputDataText;
		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring scriptText = getOutputStream(BtScript).str();
	if (!scriptText.empty())
	{
		ss << Endl;
		ss << scriptText;
		ss << Endl;
	}

	if (m_shaderType == StVertex)
	{
		ss << L"void main(InputData i";

		if (m_needInstanceID)
			ss << L", uint instanceID : SV_InstanceID";

		if (!outputDataText.empty())
			ss << L", out OutputData o";

		ss << L")" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		if (!outputDataText.empty())
			ss << L"o = (OutputData)0;" << Endl;

		ss << getOutputStream(BtBody).str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}
	else if (m_shaderType == StPixel)
	{
		ss << L"void main(";

		if (!inputDataText.empty())
			ss << L"InputData i, ";

		if (m_needInstanceID)
			ss << L"float instanceID : SV_InstanceID, ";

		if (m_needVPos)
			ss << L"float4 vPos : SV_Position, ";

		if (m_needVFace)
			ss << L"bool vFace : SV_IsFrontFace, ";

		ss << L"out OutputData o)" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << L"o = (OutputData)0;" << Endl;

		ss << getOutputStream(BtBody).str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}
	else if (m_shaderType == StCompute)
	{
		ss << L"[numthreads(1, 1, 1)]" << Endl;
		ss << L"void main(uint3 dispatchThreadID : SV_DispatchThreadID)" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << getOutputStream(BtBody).str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}

	return ss.str();
}

	}
}
