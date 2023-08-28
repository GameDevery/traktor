/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class ImageGraph;
class IRenderSystem;
class IRenderTargetSet;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::world
{

struct WorldCreateDesc;

class Entity;
class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class VelocityPass : public Object
{
    T_RTTI_CLASS;

public:
    explicit VelocityPass(
        const WorldRenderSettings& settings,
        WorldEntityRenderers* entityRenderers,
        render::IRenderTargetSet* sharedDepthStencil)
    ;

    bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

    DoubleBufferedTarget setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		const GatherView& gatheredView,
        uint32_t frameCount,
		render::RenderGraph& renderGraph,
        render::handle_t gbufferTargetSetId,
		render::handle_t outputTargetSetId
	) const;

private:
    WorldRenderSettings m_settings;
    Ref< WorldEntityRenderers > m_entityRenderers;
    Ref< render::IRenderTargetSet > m_sharedDepthStencil;
    Ref< render::ScreenRenderer > m_screenRenderer;
    resource::Proxy< render::ImageGraph > m_velocityPrime;
};

}
