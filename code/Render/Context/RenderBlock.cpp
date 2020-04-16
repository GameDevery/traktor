#include "Render/Context/RenderBlock.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/IRenderView.h"

#if defined(_DEBUG)
#	define T_CONTEXT_PUSH_MARKER(renderView, name) // T_RENDER_PUSH_MARKER(renderView, name)
#	define T_CONTEXT_POP_MARKER(renderView) // T_RENDER_POP_MARKER(renderView)
#else
#	define T_CONTEXT_PUSH_MARKER(renderView, name)
#	define T_CONTEXT_POP_MARKER(renderView)
#endif

namespace traktor
{
	namespace render
	{

void NullRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	T_CONTEXT_POP_MARKER(renderView);
}

void SimpleRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		primitives
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void InstancingRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		primitives,
		count
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedInstancingRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		p,
		instanceCount
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void NonIndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		0,
		program,
		p
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void IndexedRenderBlock::render(IRenderView* renderView) const
{
	Primitives p(primitive, offset, count, minIndex, maxIndex);

	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (programParams)
		programParams->fixup(program);

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		program,
		p
	);

	T_CONTEXT_POP_MARKER(renderView);
}

void BeginPassRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	if (renderTargetSet)
	{
		if (renderTargetIndex >= 0)
			renderView->beginPass(renderTargetSet, renderTargetIndex, &clear, load, store);
		else
			renderView->beginPass(renderTargetSet, &clear, load, store);
	}
	else
		renderView->beginPass(&clear);

	T_CONTEXT_POP_MARKER(renderView);
}

void EndPassRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->endPass();

	T_CONTEXT_POP_MARKER(renderView);
}

void PresentRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->present();

	T_CONTEXT_POP_MARKER(renderView);
}

void SetViewportRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	renderView->setViewport(viewport);

	T_CONTEXT_POP_MARKER(renderView);
}

void LambdaRenderBlock::render(IRenderView* renderView) const
{
	T_CONTEXT_PUSH_MARKER(renderView, name);

	lambda(renderView);

	T_CONTEXT_POP_MARKER(renderView);
}

	}
}
