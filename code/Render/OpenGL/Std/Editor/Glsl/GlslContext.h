#pragma once

#include <map>
#include <vector>
#include "Render/OpenGL/Std/TypesOpenGL.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslEmitter.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslShader.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslType.h"

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class GlslVariable;

/*!
 * \ingroup OGL
 */
class GlslContext
{
public:
	GlslContext(const ShaderGraph* shaderGraph, const PropertyGroup* settings);

	Node* getInputNode(const InputPin* inputPin) const;

	Node* getInputNode(Node* node, const std::wstring& inputPinName) const;

	bool emit(Node* node);

	GlslVariable* emitInput(const InputPin* inputPin);

	GlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	GlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, GlslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, GlslVariable* variable);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, AlignedVector< const OutputPin* >& outOutputPins) const;

	void enterVertex();

	void enterFragment();

	void enterCompute();

	bool inVertex() const;

	bool inFragment() const;

	bool inCompute() const;

	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	GlslRequirements& requirements();

	const GlslRequirements& requirements() const;

	const PropertyGroup* getSettings() const;

	GlslShader& getVertexShader();

	GlslShader& getFragmentShader();

	GlslShader& getComputeShader();

	GlslShader& getShader();

	GlslEmitter& getEmitter();

	RenderStateOpenGL& getRenderState();

	void defineTexture(const std::wstring& texture);

	bool defineUniform(const std::wstring& name, GLenum type, GLuint length);

	bool defineSampler(const std::wstring& name, uint32_t stateHash, GLenum target, const std::wstring& texture, int32_t& outStage);

	const std::vector< std::wstring >& getTextures() const;

	const std::vector< NamedUniformType >& getUniforms() const;

	const std::vector< SamplerBindingOpenGL >& getSamplers() const;

	const std::wstring& getError() const { return m_error; }

private:
	struct Scope
	{
		const InputPin* inputPin;
		const OutputPin* outputPin;

		Scope()
		:	inputPin(0)
		,	outputPin(0)
		{
		}

		Scope(const InputPin* inputPin_, const OutputPin* outputPin_)
		:	inputPin(inputPin_)
		,	outputPin(outputPin_)
		{
		}
	};

	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const PropertyGroup > m_settings;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader m_computeShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	RenderStateOpenGL m_renderState;
	int32_t m_nextStage;
	GlslRequirements m_requirements;
	std::vector< uint8_t > m_interpolatorMap;
	std::vector< std::wstring > m_textures;
	std::vector< NamedUniformType > m_uniforms;
	std::vector< SamplerBindingOpenGL > m_samplers;
	std::vector< uint32_t > m_samplerStateHashes;
	std::list< Scope > m_emitScope;
	std::wstring m_error;
};

	}
}
