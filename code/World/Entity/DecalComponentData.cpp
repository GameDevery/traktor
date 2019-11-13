#include "Core/Serialization/AttributeRange.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "World/Entity/DecalComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DecalComponentData", 0, DecalComponentData, IEntityComponentData)

DecalComponentData::DecalComponentData()
:	m_size(1.0f)
,	m_thickness(1.0f)
,	m_alpha(2.0f)
,	m_cullDistance(100.0f)
{
}

void DecalComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"size", m_size, AttributeRange(0.0f));
	s >> Member< float >(L"thickness", m_thickness, AttributeRange(0.0f));
	s >> Member< float >(L"alpha", m_alpha, AttributeRange(0.0f));
	s >> Member< float >(L"cullDistance", m_cullDistance, AttributeRange(0.0f));
	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

	}
}
