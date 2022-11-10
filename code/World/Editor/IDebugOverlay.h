/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class RenderGraph;
class ScreenRenderer;

    }

    namespace resource
    {

class IResourceManager;

    }

    namespace world
    {

class IWorldRenderer;
class WorldRenderView;

class T_DLLCLASS IDebugOverlay : public Object
{
    T_RTTI_CLASS;

public:
    virtual bool create(resource::IResourceManager* resourceManager) = 0;

    virtual void setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha) const = 0;
};

    }
}