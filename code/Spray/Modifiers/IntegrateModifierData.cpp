/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/IntegrateModifier.h"
#include "Spray/Modifiers/IntegrateModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.IntegrateModifierData", 1, IntegrateModifierData, ModifierData)

IntegrateModifierData::IntegrateModifierData()
:	m_timeScale(1.0f)
,	m_linear(true)
,	m_angular(true)
{
}

Ref< const Modifier > IntegrateModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if ((m_linear || m_angular) && abs(m_timeScale) > FUZZY_EPSILON)
		return new IntegrateModifier(m_timeScale, m_linear, m_angular);
	else
		return 0;
}

void IntegrateModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"timeScale", m_timeScale);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"linear", m_linear);
		s >> Member< bool >(L"angular", m_angular);
	}
}

	}
}
