/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class ITexture;
class Shader;

}

namespace traktor::weather
{

class SkyComponent;

/*! Sky background component data.
 * \ingroup Weather
 */
class T_DLLCLASS SkyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SkyComponentData();

	Ref< SkyComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

private:
	resource::Id< render::Shader > m_shader;
	resource::Id< render::ITexture > m_texture;
};

}
