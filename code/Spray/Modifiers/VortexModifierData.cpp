#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/VortexModifier.h"
#include "Spray/Modifiers/VortexModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.VortexModifierData", 0, VortexModifierData, ModifierData)

VortexModifierData::VortexModifierData()
:	m_axis(0.0f, 1.0f, 0.0f, 0.0f)
,	m_tangentForce(0.0f)
,	m_normalConstantForce(0.0f)
,	m_normalDistance(0.0f)
,	m_normalDistanceForce(0.0f)
,	m_world(true)
{
}

Ref< Modifier > VortexModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new VortexModifier(
		m_axis,
		m_tangentForce,
		m_normalConstantForce,
		m_normalDistance,
		m_normalDistanceForce,
		m_world
	);
}

bool VortexModifierData::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
	s >> Member< float >(L"tangentForce", m_tangentForce);
	s >> Member< float >(L"normalConstantForce", m_normalConstantForce);
	s >> Member< float >(L"normalDistance", m_normalDistance);
	s >> Member< float >(L"normalDistanceForce", m_normalDistanceForce);
	s >> Member< bool >(L"world", m_world);
	return true;
}

	}
}
