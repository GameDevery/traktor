#ifndef traktor_world_EntityBuilderWithSchema_H
#define traktor_world_EntityBuilderWithSchema_H

#include <list>
#include <map>
#include <stack>
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

class IEntitySchema;

/*! \brief Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilderWithSchema : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityBuilderWithSchema(IEntityBuilder* entityBuilder, IEntitySchema* entitySchema);

	EntityBuilderWithSchema(IEntityBuilder* entityBuilder, IEntitySchema* entitySchema, std::map< const world::EntityData*, Ref< world::Entity > >& outEntityProducts);

	virtual void addFactory(const IEntityFactory* entityFactory);

	virtual void removeFactory(const IEntityFactory* entityFactory);

	virtual const IEntityFactory* getFactory(const EntityData* entityData) const;

	virtual const IEntityBuilder* getCompositeEntityBuilder() const;

	virtual Ref< Entity > create(const EntityData* entityData) const;

private:
	typedef std::list< std::pair< std::wstring, Ref< Entity > > > scope_t;

	Ref< IEntityBuilder > m_entityBuilder;
	Ref< IEntitySchema > m_entitySchema;
	std::map< const world::EntityData*, Ref< world::Entity > >* m_outEntityProducts;
	mutable std::stack< scope_t > m_entityScope;
};

	}
}

#endif	// traktor_world_EntityBuilderWithSchema_H
