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
#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*!
 * \ingroup World
 */
class T_DLLCLASS DecalEventData : public IEntityEventData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	float getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	float m_size = 1.0f;
	float m_thickness = 1.0f;
	float m_alpha = 2.0f;
	float m_cullDistance = 100.0f;
	resource::Id< render::Shader > m_shader;
};

}
