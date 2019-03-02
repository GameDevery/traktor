#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
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

/*! \brief Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilder : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	virtual void addFactory(const IEntityFactory* entityFactory) override final;

	virtual void removeFactory(const IEntityFactory* entityFactory) override final;

	virtual const IEntityFactory* getFactory(const EntityData* entityData) const override final;

	virtual const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const override final;

	virtual const IEntityFactory* getFactory(const IEntityComponentData* entityComponentData) const override final;

	virtual Ref< Entity > create(const EntityData* entityData) const override final;

	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const override final;

	virtual Ref< IEntityComponent > create(const IEntityComponentData* entityComponentData) const override final;

	virtual const IEntityBuilder* getCompositeEntityBuilder() const override final;

private:
	mutable Semaphore m_lock;
	RefArray< const IEntityFactory > m_entityFactories;
	mutable std::map< const TypeInfo*, const IEntityFactory* > m_resolvedFactoryCache;
};

	}
}

