/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{4CF929EB-3A8B-C340-AA0A-0C5C80625BF1}"));
const resource::Id< render::ITexture > c_defaultTexture(Guid(L"{93E6996B-8903-4AD0-811A-C8C03C8E38C6}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.SkyComponentData", 4, SkyComponentData, world::IEntityComponentData)

SkyComponentData::SkyComponentData()
:	m_shader(c_defaultShader)
,	m_texture(c_defaultTexture)
,	m_intensity(1.0f)
{
}

Ref< SkyComponent > SkyComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;
		
	resource::Proxy< render::ITexture > texture;
	if (m_texture.isValid())
	{
		if (!resourceManager->bind(m_texture, texture))
			return nullptr;
	}

	Ref< SkyComponent > skyComponent = new SkyComponent(
		shader,
		texture,
		m_intensity
	);
	skyComponent->create(resourceManager, renderSystem);
	return skyComponent;
}

int32_t SkyComponentData::getOrdinal() const
{
	return 0;
}

void SkyComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SkyComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader, AttributePrivate());

	if (s.getVersion< SkyComponentData >() >= 2)
		s >> resource::Member< render::ITexture >(L"texture", m_texture);

	if (s.getVersion< SkyComponentData >() < 1)
	{
		Vector4 dummy;
		s >> Member< Vector4 >(L"sunDirection", dummy);
	}

	if (s.getVersion< SkyComponentData >() < 3)
	{
		float dummy;
		s >> Member< float >(L"offset", dummy);
	}

	if (s.getVersion< SkyComponentData >() >= 4)
		s >> Member< float >(L"intensity", m_intensity, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
}

}
