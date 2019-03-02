#pragma once

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityAdapterBuilder(
		SceneEditorContext* context,
		world::IEntityBuilder* entityBuilder,
		EntityAdapter* currentEntityAdapter
	);

	virtual ~EntityAdapterBuilder();

	virtual void addFactory(const world::IEntityFactory* entityFactory) override final;

	virtual void removeFactory(const world::IEntityFactory* entityFactory) override final;

	virtual const world::IEntityFactory* getFactory(const world::EntityData* entityData) const override final;

	virtual const world::IEntityFactory* getFactory(const world::IEntityEventData* entityEventData) const override final;

	virtual const world::IEntityFactory* getFactory(const world::IEntityComponentData* entityComponentData) const override final;

	virtual Ref< world::Entity > create(const world::EntityData* entityData) const override final;

	virtual Ref< world::IEntityEvent > create(const world::IEntityEventData* entityEventData) const override final;

	virtual Ref< world::IEntityComponent > create(const world::IEntityComponentData* entityComponentData) const override final;

	virtual const world::IEntityBuilder* getCompositeEntityBuilder() const override final;

	EntityAdapter* getRootAdapter() const { return m_rootAdapter; }

	uint32_t getCacheHit() const { return m_cacheHit; }

	uint32_t getCacheMiss() const { return m_cacheMiss; }

private:
	struct Cache
	{
		RefArray< EntityAdapter > adapters;
		std::map< uint32_t, RefArray< world::Entity > > leafEntities;
	};

	SceneEditorContext* m_context;
	Ref< world::IEntityBuilder > m_entityBuilder;
	mutable std::map< const TypeInfo*, Cache > m_cache;
	mutable Ref< EntityAdapter > m_currentAdapter;
	mutable Ref< EntityAdapter > m_rootAdapter;
	mutable uint32_t m_cacheHit;
	mutable uint32_t m_cacheMiss;
};

	}
}

