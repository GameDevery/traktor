#include <cassert>
#include "Core/Log/Log.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Editor/Hlsl/HlslContext.h"
#include "Render/Dx11/Editor/Hlsl/HlslShader.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

std::wstring getClassNameOnly(const Object* o)
{
	std::wstring qn = type_name(o);
	size_t p = qn.find_last_of('.');
	return qn.substr(p + 1);
}

		}

HlslContext::HlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(HlslShader::StVertex)
,	m_pixelShader(HlslShader::StPixel)
,	m_computeShader(HlslShader::StCompute)
,	m_currentShader(nullptr)
,	m_stencilReference(0)
{
	std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	m_d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	m_d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	m_d3dRasterizerDesc.DepthBias = 0;
	m_d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	m_d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_d3dRasterizerDesc.DepthClipEnable = TRUE;
	m_d3dRasterizerDesc.ScissorEnable = FALSE;
	m_d3dRasterizerDesc.MultisampleEnable = FALSE;
	m_d3dRasterizerDesc.AntialiasedLineEnable = TRUE;

	std::memset(&m_d3dDepthStencilDesc, 0, sizeof(m_d3dDepthStencilDesc));
	m_d3dDepthStencilDesc.DepthEnable = TRUE;
	m_d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_d3dDepthStencilDesc.StencilEnable = FALSE;

	std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
	m_d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_d3dBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	m_d3dBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	for (int i = 0; i < sizeof_array(m_d3dBlendDesc.RenderTarget); ++i)
		m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

Node* HlslContext::getInputNode(const InputPin* inputPin) const
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : nullptr;
}

Node* HlslContext::getInputNode(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT(inputPin);

	return getInputNode(inputPin);
}

bool HlslContext::emit(Node* node)
{
	// In case we're in failure state we ignore recursing further.
	if (!m_error.empty())
		return false;

	bool allOutputsEmitted = true;

	// Check if all active outputs of node already has been emitted.
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		T_ASSERT(outputPin != nullptr);

		if (m_shaderGraph->getDestinationCount(outputPin) == 0)
			continue;

		HlslVariable* variable = m_currentShader->getVariable(node->getOutputPin(i));
		if (!variable)
		{
			allOutputsEmitted = false;
			break;
		}
	}
	if (outputPinCount > 0 && allOutputsEmitted)
		return true;

	return m_emitter.emit(*this, node);
}

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	// In case we're in failure state we ignore recursing further.
	if (!m_error.empty())
		return nullptr;

	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return nullptr;

	// Check if node's output already has been emitted.
	Ref< HlslVariable > variable = m_currentShader->getVariable(sourcePin);
	if (variable)
		return variable;

	Node* node = sourcePin->getNode();

	m_emitScope.push_back(Scope(
		inputPin,
		sourcePin
	));

	bool result = m_emitter.emit(*this, node);
	if (result)
	{
		variable = m_currentShader->getVariable(sourcePin);
		T_ASSERT(variable);
	}
	else
	{
		// Only log first failure point; all recursions will also fail.
		if (m_error.empty())
		{
			// Format chain to properly indicate source of error.
			StringOutputStream ss;
			for (std::list< Scope >::const_reverse_iterator i = m_emitScope.rbegin(); i != m_emitScope.rend(); ++i)
				ss << getClassNameOnly(i->outputPin->getNode()) << L"[" << i->outputPin->getName() << L"] <-- [" << i->inputPin->getName() << L"]";
			ss << getClassNameOnly(m_emitScope.front().inputPin->getNode());
			m_error = ss.str();
		}
	}

	m_emitScope.pop_back();
	return variable;
}

HlslVariable* HlslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT(inputPin);
	return emitInput(inputPin);
}

HlslVariable* HlslContext::emitOutput(const OutputPin* outputPin, HlslType type)
{
	HlslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT(out);
	return out;
}

HlslVariable* HlslContext::emitOutput(Node* node, const std::wstring& outputPinName, HlslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT(outputPin);
	return emitOutput(outputPin, type);
}

void HlslContext::findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins) const
{
	getNonDependentOutputs(m_shaderGraph, node->findInputPin(inputPinName), dependentOutputPins, outOutputPins);
}

void HlslContext::findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, AlignedVector< const OutputPin* >& outOutputPins) const
{
	AlignedVector< const InputPin* > inputPins(2);
	inputPins[0] = node->findInputPin(inputPin1);
	inputPins[1] = node->findInputPin(inputPin2);
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins);
}

void HlslContext::findCommonOutputs(const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outOutputPins) const
{
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins);
}

void HlslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void HlslContext::enterPixel()
{
	m_currentShader = &m_pixelShader;
}

void HlslContext::enterCompute()
{
	m_currentShader = &m_computeShader;
}

bool HlslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool HlslContext::inPixel() const
{
	return bool(m_currentShader == &m_pixelShader);
}

bool HlslContext::inCompute() const
{
	return bool(m_currentShader == &m_computeShader);
}

bool HlslContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
{
	int32_t lastId = int32_t(m_interpolatorMap.size());

	for (int32_t i = 0; i < lastId; ++i)
	{
		uint8_t& occupied = m_interpolatorMap[i];
		if (width <= 4 - occupied)
		{
			outId = i;
			outOffset = occupied;
			occupied += width;
			return false;
		}
	}

	outId = lastId;
	outOffset = 0;

	m_interpolatorMap.push_back(width);
	return true;
}

HlslShader& HlslContext::getVertexShader()
{
	return m_vertexShader;
}

HlslShader& HlslContext::getPixelShader()
{
	return m_pixelShader;
}

HlslShader& HlslContext::getComputeShader()
{
	return m_computeShader;
}

HlslShader& HlslContext::getShader()
{
	T_ASSERT(m_currentShader);
	return *m_currentShader;
}

HlslEmitter& HlslContext::getEmitter()
{
	return m_emitter;
}

D3D11_RASTERIZER_DESC& HlslContext::getD3DRasterizerDesc()
{
	return m_d3dRasterizerDesc;
}

D3D11_DEPTH_STENCIL_DESC& HlslContext::getD3DDepthStencilDesc()
{
	return m_d3dDepthStencilDesc;
}

D3D11_BLEND_DESC& HlslContext::getD3DBlendDesc()
{
	return m_d3dBlendDesc;
}

void HlslContext::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

uint32_t HlslContext::getStencilReference() const
{
	return m_stencilReference;
}

	}
}
