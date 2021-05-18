#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class Blob;
class ProgramDx11;

class StateCache : public Object
{
public:
	StateCache(ID3D11DeviceContext* d3dDeviceContext);

	void reset();

	void setRasterizerState(ID3D11RasterizerState* d3dRasterizerState);

	void setDepthStencilState(ID3D11DepthStencilState* d3dDepthStencilState, UINT stencilReference);

	void setBlendState(ID3D11BlendState* d3dBlendState);

	void setVertexShader(ID3D11VertexShader* d3dVertexShader);

	void setPixelShader(ID3D11PixelShader* d3dPixelShader);

	void setVertexBuffer(ID3D11Buffer* d3dVertexBuffer, UINT d3dVertexStride);

	void setIndexBuffer(ID3D11Buffer* d3dIndexBuffer, DXGI_FORMAT d3dIndexFormat);

	void setTopology(D3D11_PRIMITIVE_TOPOLOGY d3dTopology);

	void setInputLayout(uint32_t d3dVertexShaderHash, const Blob* d3dVertexShaderBlob, uint32_t d3dInputElementsHash, const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements);

	void setActiveProgram(ProgramDx11* program);

	ProgramDx11* getActiveProgram() const;

private:
	ComRef< ID3D11DeviceContext > m_d3dDeviceContext;
	ComRef< ID3D11RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D11DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D11BlendState > m_d3dBlendState;
	ComRef< ID3D11VertexShader > m_d3dVertexShader;
	ComRef< ID3D11PixelShader > m_d3dPixelShader;
	ComRef< ID3D11Buffer > m_d3dVertexBuffer;
	ComRef< ID3D11Buffer > m_d3dIndexBuffer;
	SmallMap< uint64_t, ComRef< ID3D11InputLayout > > m_d3dInputLayouts;
	D3D11_PRIMITIVE_TOPOLOGY m_d3dTopology;
	uint64_t m_d3dSignatureHash;
	uint32_t m_stencilReference;
	ProgramDx11* m_activeProgram;
};

	}
}

