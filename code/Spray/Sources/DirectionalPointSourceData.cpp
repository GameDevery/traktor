#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/DirectionalPointSource.h"
#include "Spray/Sources/DirectionalPointSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DirectionalPointSourceData", 0, DirectionalPointSourceData, SourceData)

DirectionalPointSourceData::DirectionalPointSourceData()
:	SourceData()
,	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_direction(0.0f, 1.0f, 0.0f, 0.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

Ref< Source > DirectionalPointSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new DirectionalPointSource(
		getConstantRate(),
		getVelocityRate(),
		m_position,
		m_direction,
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

bool DirectionalPointSourceData::serialize(ISerializer& s)
{
	if (!SourceData::serialize(s))
		return false;

	s >> Member< Vector4 >(L"position", m_position, AttributePoint());
	s >> Member< Vector4 >(L"direction", m_direction, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);

	return true;
}

	}
}
