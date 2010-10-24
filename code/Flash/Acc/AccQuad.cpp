#include "Core/Math/Matrix44.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/SwfTypes.h"
#include "Flash/Acc/AccQuad.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_guidShaderSolid(L"{1EDAAA67-1E02-8A49-B857-14D7812C96D6}");
const Guid c_guidShaderTextured(L"{10426D17-CF0A-4849-A207-24F101A78459}");
const Guid c_guidShaderSolidMask(L"{2EDC5E1B-562D-9F46-9E3C-474729FB078E}");
const Guid c_guidShaderTexturedMask(L"{98A59F6A-1D90-144C-B688-4CEF382453F2}");

#pragma pack(1)
struct Vertex
{
	float pos[2];
};
#pragma pack()

bool s_handleInitialized = false;
render::handle_t s_handleTransform;
render::handle_t s_handleFrameSize;
render::handle_t s_handleViewSize;
render::handle_t s_handleViewOffset;
render::handle_t s_handleScreenOffsetScale;
render::handle_t s_handleCxFormMul;
render::handle_t s_handleCxFormAdd;
render::handle_t s_handleTexture;
render::handle_t s_handleTextureOffset;

		}

bool AccQuad::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!s_handleInitialized)
	{
		s_handleTransform = render::getParameterHandle(L"Transform");
		s_handleFrameSize = render::getParameterHandle(L"FrameSize");
		s_handleViewSize = render::getParameterHandle(L"ViewSize");
		s_handleViewOffset = render::getParameterHandle(L"ViewOffset");
		s_handleScreenOffsetScale = render::getParameterHandle(L"ScreenOffsetScale");
		s_handleCxFormMul = render::getParameterHandle(L"CxFormMul");
		s_handleCxFormAdd = render::getParameterHandle(L"CxFormAdd");
		s_handleTexture = render::getParameterHandle(L"Texture");
		s_handleTextureOffset = render::getParameterHandle(L"TextureOffset");
		s_handleInitialized = true;
	}

	m_shaderSolid = c_guidShaderSolid;
	if (!resourceManager->bind(m_shaderSolid))
		return false;

	m_shaderTextured = c_guidShaderTextured;
	if (!resourceManager->bind(m_shaderTextured))
		return false;

	m_shaderSolidMask = c_guidShaderSolidMask;
	if (!resourceManager->bind(m_shaderSolidMask))
		return false;

	m_shaderTexturedMask = c_guidShaderTexturedMask;
	if (!resourceManager->bind(m_shaderTexturedMask))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 2 * 3 * sizeof(Vertex), false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	vertex->pos[0] = 0.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 0.0f; vertex->pos[1] = 1.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 1.0f; ++vertex;

	m_vertexBuffer->unlock();

	return true;
}

void AccQuad::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void AccQuad::render(
	render::RenderContext* renderContext,
	const SwfRect& bounds,
	const Matrix33& transform,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	float screenOffsetScale,
	const SwfCxTransform& cxform,
	render::ITexture* texture,
	const Vector4& textureOffset,
	uint8_t maskReference
)
{
	Matrix44 m1(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m2(
		bounds.max.x - bounds.min.x, 0.0f, bounds.min.x, 0.0f,
		0.0f, bounds.max.y - bounds.min.y, bounds.min.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;

	Ref< render::Shader > shaderSolid, shaderTextured;
	if (maskReference == 0)
	{
		shaderSolid = m_shaderSolid;
		shaderTextured = m_shaderTextured;
	}
	else
	{
		shaderSolid = m_shaderSolidMask;
		shaderTextured = m_shaderTexturedMask;
	}

	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccQuad");
	renderBlock->program = (texture ? shaderTextured : shaderSolid)->getCurrentProgram();
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setMatrixParameter(s_handleTransform, m);
	renderBlock->programParams->setVectorParameter(s_handleFrameSize, frameSize);
	renderBlock->programParams->setVectorParameter(s_handleViewSize, viewSize);
	renderBlock->programParams->setVectorParameter(s_handleViewOffset, viewOffset);
	renderBlock->programParams->setFloatParameter(s_handleScreenOffsetScale, screenOffsetScale);
	renderBlock->programParams->setVectorParameter(s_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
	renderBlock->programParams->setVectorParameter(s_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
	renderBlock->programParams->setStencilReference(maskReference);
	
	if (texture)
	{
		renderBlock->programParams->setTextureParameter(s_handleTexture, texture);
		renderBlock->programParams->setVectorParameter(s_handleTextureOffset, textureOffset);
	}

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RfOverlay, renderBlock);
}

	}
}
