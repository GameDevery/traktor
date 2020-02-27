#pragma once

#include <list>
#include <stack>
#include <string>
#include "Core/Containers/SmallMap.h"
#include "World/IEntityBuilder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntitySchema;

/*! Entity builder with recording schema.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilderWithSchema : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityBuilderWithSchema(IEntityBuilder* entityBuilder, EntitySchema* entitySchema);

	EntityBuilderWithSchema(IEntityBuilder* entityBuilder, EntitySchema* entitySchema, SmallMap< const world::EntityData*, Ref< world::Entity > >& outEntityProducts);

	virtual void addFactory(const IEntityFactory* entityFactory) override;

	virtual void removeFactory(const IEntityFactory* entityFactory) override;

	virtual const IEntityFactory* getFactory(const EntityData* entityData) const override;

	virtual const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const override;

	virtual const IEntityFactory* getFactory(const IEntityComponentData* entityComponentData) const override;

	virtual Ref< Entity > create(const EntityData* entityData) const override;

	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const override;

	virtual Ref< IEntityComponent > create(const IEntityComponentData* entityComponentData) const override;

	virtual const IEntityBuilder* getCompositeEntityBuilder() const override;

private:
	typedef std::list< std::pair< std::wstring, Ref< Entity > > > scope_t;

	Ref< IEntityBuilder > m_entityBuilder;
	Ref< EntitySchema > m_entitySchema;
	SmallMap< const world::EntityData*, Ref< world::Entity > >* m_outEntityProducts;
	mutable std::stack< scope_t > m_entityScope;
};

	}
}

