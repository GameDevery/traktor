/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/VelocityPass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_velocityPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VelocityPass", VelocityPass, Object)

VelocityPass::VelocityPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers,
    render::IRenderTargetSet* sharedDepthStencil
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
,   m_sharedDepthStencil(sharedDepthStencil)
{
}

bool VelocityPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create velocity prime processing; priming is also used by TAA.
	if (
		desc.quality.motionBlur > Quality::Disabled ||
		desc.quality.antiAlias >= Quality::Ultra
	)
	{
		if (!resourceManager->bind(c_velocityPrime, m_velocityPrime))
		{
			log::error << L"Unable to create velocity prime process." << Endl;
			return false;
		}
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::handle_t VelocityPass::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
    const GatherView& gatheredView,
	render::ImageGraphContext* imageGraphContext,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"VelocityPass::setup");

	// Add Velocity target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32G32F;
	auto velocityTargetSetId = renderGraph.addTransientTargetSet(L"Velocity", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add Velocity render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Velocity");

	if (m_velocityPrime)
	{
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getLastView() * worldRenderView.getView().inverse();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = worldRenderView.getDeltaTime();

		imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);

		m_velocityPrime->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			*imageGraphContext,
			view
		);
	}

	rp->setOutput(velocityTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared velocityPass(
				s_techniqueVelocityWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::None
			);

			for (auto r : gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, velocityPass, r.renderable);

			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, velocityPass);
		}
	);

	renderGraph.addPass(rp);
	return velocityTargetSetId;
}

}