/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/VelocityPass.h"

#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::ImageGraph > c_velocityPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VelocityPass", VelocityPass, Object)

VelocityPass::VelocityPass(WorldEntityRenderers* entityRenderers)
	: m_entityRenderers(entityRenderers)
{
}

bool VelocityPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create velocity prime processing.
	if (!resourceManager->bind(c_velocityPrime, m_velocityPrime))
	{
		log::error << L"Unable to create velocity prime process." << Endl;
		return false;
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::RGTargetSet VelocityPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"VelocityPass::setup");

	const bool rayTracingEnable = (bool)(gatheredView.rtWorldTopLevel != nullptr);

	// Add Velocity target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16F;
	const render::RGTargetSet velocityTargetSetId = renderGraph.addTransientTargetSet(L"Velocity", rgtd, outputTargetSetId, outputTargetSetId);

	// Add Velocity render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Velocity");
	rp->addInput(gbufferTargetSetId);
	rp->setOutput(velocityTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	if (m_velocityPrime)
	{
		render::ImageGraphView view;
		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getLastView() * worldRenderView.getView().inverse();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = (float)worldRenderView.getDeltaTime();

		render::ImageGraphContext igctx;
		igctx.setTechniqueFlag(ShaderPermutation::RayTracingEnable, rayTracingEnable);

		auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			params->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
			params->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
			params->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));
		};

		m_velocityPrime->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			igctx,
			view,
			setParameters);
	}

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(
			m_entityRenderers,
			renderContext);

		auto sharedParams = renderContext->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(renderContext);
		sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
		sharedParams->endParameters(renderContext);

		const WorldRenderPassShared velocityPass(
			ShaderTechnique::VelocityWrite,
			sharedParams,
			worldRenderView,
			IWorldRenderPass::None);

		for (auto r : gatheredView.renderables)
			if (r.state.dynamic)
				r.renderer->build(wc, worldRenderView, velocityPass, r.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->build(wc, worldRenderView, velocityPass);
	});

	renderGraph.addPass(rp);
	return velocityTargetSetId;
}
}
