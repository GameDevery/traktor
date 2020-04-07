#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/UndergrowthComponentData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.UndergrowthComponentData", 1, UndergrowthComponentData, TerrainLayerComponentData)

UndergrowthComponentData::UndergrowthComponentData()
:	m_spreadDistance(100.0f)
{
}

void UndergrowthComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> MemberAlignedVector< Plant, MemberComposite< Plant > >(L"plants", m_plants);
}

UndergrowthComponentData::Plant::Plant()
:	attribute(1)
,	density(100)
,	plant(0)
,	scale(1.0f)
{
}

void UndergrowthComponentData::Plant::serialize(ISerializer& s)
{
	if (s.getVersion< UndergrowthComponentData >() >= 1)
		s >> Member< uint8_t >(L"attribute", attribute);
	else
		s >> Member< uint8_t >(L"material", attribute);

	s >> Member< int32_t >(L"density", density);
	s >> Member< int32_t >(L"plant", plant);
	s >> Member< float >(L"scale", scale);
}

	}
}
