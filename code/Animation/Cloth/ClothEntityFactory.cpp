#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/Cloth/ClothEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityFactory", ClothEntityFactory, world::IEntityFactory)

ClothEntityFactory::ClothEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ClothEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< ClothComponentData >();
}

const TypeInfoSet ClothEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet ClothEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > ClothEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > ClothEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > ClothEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return checked_type_cast< const ClothComponentData* >(&entityComponentData)->createComponent(m_resourceManager, m_renderSystem);
}

	}
}
