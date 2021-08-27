#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/IVertexLayout.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#pragma pack(1)
struct ScreenVertex
{
	float pos[2];
	float texCoord[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScreenRenderer", ScreenRenderer, Object)

bool ScreenRenderer::create(IRenderSystem* renderSystem)
{
	AlignedVector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DuPosition, DtFloat2, offsetof(ScreenVertex, pos)));
	vertexElements.push_back(VertexElement(DuCustom, DtFloat2, offsetof(ScreenVertex, texCoord)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);
	if (!m_vertexLayout)
		return false;

	m_vertexBuffer = renderSystem->createBuffer(BuVertex, 6, sizeof(ScreenVertex), false);
	if (!m_vertexBuffer)
		return false;

	ScreenVertex* vertex = reinterpret_cast< ScreenVertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	vertex[2].pos[0] = -1.0f; vertex[2].pos[1] =  1.0f; vertex[2].texCoord[0] = 0.0f; vertex[2].texCoord[1] = 0.0f;
	vertex[1].pos[0] =  1.0f; vertex[1].pos[1] =  1.0f; vertex[1].texCoord[0] = 1.0f; vertex[1].texCoord[1] = 0.0f;
	vertex[0].pos[0] =  1.0f; vertex[0].pos[1] = -1.0f; vertex[0].texCoord[0] = 1.0f; vertex[0].texCoord[1] = 1.0f;

	vertex[5].pos[0] = -1.0f; vertex[5].pos[1] =  1.0f; vertex[5].texCoord[0] = 0.0f; vertex[5].texCoord[1] = 0.0f;
	vertex[4].pos[0] =  1.0f; vertex[4].pos[1] = -1.0f; vertex[4].texCoord[0] = 1.0f; vertex[4].texCoord[1] = 1.0f;
	vertex[3].pos[0] = -1.0f; vertex[3].pos[1] = -1.0f; vertex[3].texCoord[0] = 0.0f; vertex[3].texCoord[1] = 1.0f;

	m_vertexBuffer->unlock();

	m_primitives.setNonIndexed(PtTriangles, 0, 2);
	return true;
}

void ScreenRenderer::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void ScreenRenderer::draw(IRenderView* renderView, IProgram* program)
{
	if (program)
		renderView->draw(m_vertexBuffer->getBufferView(), m_vertexLayout, nullptr, ItVoid, program, m_primitives, 1);
}

void ScreenRenderer::draw(IRenderView* renderView, const Shader* shader)
{
	IProgram* program = shader->getProgram().program;
	if (program)
		draw(renderView, program);
}

void ScreenRenderer::draw(IRenderView* renderView, const Shader* shader, const Shader::Permutation& permutation)
{
	if (!shader)
		return;

	IProgram* program = shader->getProgram(permutation).program;
	draw(renderView, program);
}

void ScreenRenderer::draw(RenderContext* renderContext, IProgram* program, ProgramParameters* programParams)
{
	if (!program)
		return;

	auto rb = renderContext->alloc< SimpleRenderBlock >(T_FILE_LINE_W);
	rb->program = program;
	rb->programParams = programParams;
	rb->indexBuffer = nullptr;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitives = m_primitives;
	renderContext->enqueue(rb);
}

void ScreenRenderer::draw(RenderContext* renderContext, const Shader* shader, ProgramParameters* programParams)
{
	if (!shader)
		return;

	IProgram* program = shader->getProgram().program;
	draw(renderContext, program, programParams);
}

void ScreenRenderer::draw(RenderContext* renderContext, const Shader* shader, const Shader::Permutation& permutation, ProgramParameters* programParams)
{
	if (!shader)
		return;

	IProgram* program = shader->getProgram(permutation).program;
	if (program)
		draw(renderContext, program, programParams);
}

	}
}
