#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Scene/Editor/EntityClipboardData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.EntityClipboardData", 0, EntityClipboardData, ISerializable)

void EntityClipboardData::addEntityData(world::EntityData* entityData)
{
	m_entityData.push_back(entityData);
}

const RefArray< world::EntityData >& EntityClipboardData::getEntityData() const
{
	return m_entityData;
}

bool EntityClipboardData::serialize(ISerializer& s)
{
	return s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
}

	}
}
