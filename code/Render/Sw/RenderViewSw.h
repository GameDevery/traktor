#pragma once

#include <cstring>
#include <vector>
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Graphics/ISurface.h"
#include "Render/IRenderView.h"
#include "Render/Sw/VaryingUtils.h"

namespace traktor
{
	namespace graphics
	{

class IGraphicsSystem;

	}

	namespace render
	{

class RenderSystemSw;
class RenderTargetSetSw;
class VertexBufferSw;
class IndexBufferSw;
class ProgramSw;
class Processor;

/*!
 * \ingroup SW
 */
class RenderViewSw : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewSw(RenderSystemSw* renderSystem, graphics::IGraphicsSystem* graphicsSystem, Processor* processor);

	virtual ~RenderViewSw();

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual Viewport getViewport() override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool begin(EyeType eye) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) override final;

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual void end() override final;

	virtual void present() override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	virtual bool getBackBufferContent(void* buffer) const override final;

private:
	struct RenderState
	{
		Viewport viewPort;
		uint32_t width;
		uint32_t height;
		uint32_t* colorTarget;
		uint32_t colorTargetPitch;		//< Color target pitch in bytes.
		float* depthTarget;
		uint32_t depthTargetPitch;		//< Depth target pitch in bytes.
		uint8_t* stencilTarget;
		uint32_t stencilTargetPitch;	//< Stencil target pitch in bytes.
	};

	struct FragmentContext
	{
		T_ALIGN16 varying_data_t vertexVaryings[3];			//< From vertex stream for each triangle
		T_ALIGN16 varying_data_t interpolatorVaryings[3];	//< Post vertex program
		T_ALIGN16 varying_data_t clippedVaryings[8];		//< Post view plane clip
		uint32_t clippedCount;
		Vector2 screen[8];
		const Vector2* triangle;
		uint32_t indices[3];
		float baryDenom[3];
		float baryOffset[3];
		bool depthEnable;
		bool depthWriteEnable;
		bool blendEnable;

		FragmentContext()
		:	clippedCount(0)
		,	triangle(0)
		,	depthEnable(false)
		,	depthWriteEnable(false)
		,	blendEnable(false)
		{
			std::memset(vertexVaryings, 0, sizeof(vertexVaryings));
			std::memset(interpolatorVaryings, 0, sizeof(interpolatorVaryings));
			std::memset(clippedVaryings, 0, sizeof(clippedVaryings));
		}
	};

	Ref< RenderSystemSw > m_renderSystem;
	Ref< graphics::IGraphicsSystem > m_graphicsSystem;
	Ref< Processor > m_processor;

	/*! \name Primary render target surfaces. */
	//@{
	Ref< graphics::ISurface > m_frameBufferSurface;
	graphics::SurfaceDesc m_frameBufferSurfaceDesc;
	Ref< RenderTargetSetSw > m_primaryTarget;
	//@}

	std::vector< RenderState > m_renderStateStack;
	Ref< VertexBufferSw > m_currentVertexBuffer;
	Ref< IndexBufferSw > m_currentIndexBuffer;
	Ref< ProgramSw > m_currentProgram;
	Viewport m_viewPort;
	Vector4 m_targetSize;
	int32_t m_instance;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	void fetchVertex(uint32_t index, varying_data_t& outVertexVarying) const;

	void executeVertexShader(const varying_data_t& vertexVarying, varying_data_t& outInterpolatorVarying) /*const*/;

	void clipPlanes(FragmentContext& context, uint32_t vertexCount) const;

	void projectScreen(FragmentContext& context) const;

	void drawIndexed(const Primitives& primitives);

	void drawNonIndexed(const Primitives& primitives);

	void triangleShadeOpaque(const FragmentContext& context, int x1, int x2, int y);

	void triangleShadeBlend(const FragmentContext& context, int x1, int x2, int y);
};

	}
}
