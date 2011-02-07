#ifndef traktor_render_HlslContext_H
#define traktor_render_HlslContext_H

#include "Render/Dx9/HlslEmitter.h"
#include "Render/Dx9/HlslShader.h"
#include "Render/Dx9/StateBlockDx9.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class HlslVariable;

/*!
 * \ingroup DX9 Xbox360
 */
class HlslContext
{
public:
	HlslContext(const ShaderGraph* shaderGraph);

	HlslVariable* emitInput(const InputPin* inputPin);

	HlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	HlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, HlslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, HlslVariable* variable);

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	int32_t allocateBooleanRegister();

	void allocateVPos();

	inline HlslShader& getVertexShader() { return m_vertexShader; }

	inline HlslShader& getPixelShader() { return m_pixelShader; }

	inline HlslShader& getShader() { return *m_currentShader; }

	inline HlslEmitter& getEmitter() { return m_emitter; }

	inline StateBlockDx9& getState() { return m_state; }

	inline bool needVPos() const { return m_needVPos; }

private:
	Ref< const ShaderGraph > m_shaderGraph;
	HlslShader m_vertexShader;
	HlslShader m_pixelShader;
	HlslShader* m_currentShader;
	HlslEmitter m_emitter;
	StateBlockDx9 m_state;
	std::vector< uint8_t > m_interpolatorMap;
	int32_t m_booleanRegisterCount;
	bool m_needVPos;
};

	}
}

#endif	// traktor_render_HlslContext_H
