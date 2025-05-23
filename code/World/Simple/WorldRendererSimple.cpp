/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Simple/WorldRendererSimple.h"

#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "World/Entity.h"
#include "World/Entity/RTWorldComponent.h"
#include "World/IEntityComponent.h"
#include "World/IEntityRenderer.h"
#include "World/Simple/WorldRenderPassSimple.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::world
{
namespace
{

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, float value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR32F;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc)
{
	m_entityRenderers = desc.entityRenderers;
	m_depthTexture = create1x1Texture(renderSystem, desc.worldRenderSettings->viewFarZ);
	return true;
}

void WorldRendererSimple::destroy()
{
	m_entityRenderers = nullptr;
	safeDestroy(m_depthTexture);
}

void WorldRendererSimple::setup(
	const World* world,
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet outputTargetSetId,
	const std::function< bool(const EntityState& state) >& filter)
{
	// Gather active renderables for this frame.
	{
		T_PROFILER_SCOPE(L"WorldRendererSimple gather");

		m_gathered.resize(0);
		m_gatheredTLAS = nullptr;

		for (auto component : world->getComponents())
		{
			IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
			if (entityRenderer)
				m_gathered.push_back({ entityRenderer, component });

			if (auto rtWorldComponent = dynamic_type_cast< const RTWorldComponent* >(component))
				m_gatheredTLAS = rtWorldComponent->getTopLevel();
		}

		for (auto entity : world->getEntities())
		{
			const EntityState& state = entity->getState();

			if (filter != nullptr && filter(state) == false)
				continue;
			else if (filter == nullptr && state.visible == false)
				continue;

			for (auto component : entity->getComponents())
			{
				IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
				if (entityRenderer)
					m_gathered.push_back({ entityRenderer, component });
			}
		}
	}

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererSimple setup extra passes");
		const WorldSetupContext context(world, m_entityRenderers, renderGraph, m_visualAttachments);

		for (auto gathered : m_gathered)
			gathered.entityRenderer->setup(context, worldRenderView, gathered.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add passes to render graph.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth;
	cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	cl.depth = 1.0f;
	rp->setOutput(outputTargetSetId, cl, render::TfNone, render::TfAll);

	for (auto attachment : m_visualAttachments)
		rp->addInput(attachment);

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(
			m_entityRenderers,
			renderContext);

		// Default visual parameters.
		auto globalProgramParams = renderContext->alloc< render::ProgramParameters >();
		globalProgramParams->beginParameters(renderContext);
		globalProgramParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		globalProgramParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		globalProgramParams->setTextureParameter(ShaderParameter::GBufferA, m_depthTexture);
		if (m_gatheredTLAS != nullptr)
			globalProgramParams->setAccelerationStructureParameter(ShaderParameter::TLAS, m_gatheredTLAS);
		globalProgramParams->endParameters(renderContext);

		// Build visual context.
		const WorldRenderPassSimple defaultPass(
			ShaderTechnique::SimpleColor,
			globalProgramParams,
			worldRenderView.getView());

		for (auto gathered : m_gathered)
			gathered.entityRenderer->build(wc, worldRenderView, defaultPass, gathered.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->build(wc, worldRenderView, defaultPass);
	});
	renderGraph.addPass(rp);
}

}
