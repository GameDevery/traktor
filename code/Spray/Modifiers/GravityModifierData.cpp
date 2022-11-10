/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/GravityModifier.h"
#include "Spray/Modifiers/GravityModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.GravityModifierData", 0, GravityModifierData, ModifierData)

GravityModifierData::GravityModifierData()
:	m_gravity(0.0f, -9.12f, 0.0f, 0.0f)
,	m_world(true)
{
}

Ref< const Modifier > GravityModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if (m_gravity.length2() > FUZZY_EPSILON * FUZZY_EPSILON)
		return new GravityModifier(m_gravity, m_world);
	else
		return 0;
}

void GravityModifierData::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"gravity", m_gravity, AttributeDirection());
	s >> Member< bool >(L"world", m_world);
}

	}
}
