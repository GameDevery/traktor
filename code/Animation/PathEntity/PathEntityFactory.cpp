#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityFactory", PathEntityFactory, world::IEntityFactory)

const TypeInfoSet PathEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

Ref< world::Entity > PathEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const PathEntityData* pathEntityData = checked_type_cast< const PathEntityData* >(&entityData);
	return pathEntityData->createEntity(builder);
}

	}
}
