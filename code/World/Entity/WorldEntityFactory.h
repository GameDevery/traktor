#ifndef traktor_world_WorldEntityFactory_H
#define traktor_world_WorldEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

/*! \brief World entity factory.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	WorldEntityFactory(resource::IResourceManager* resourceManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const;

private:
	mutable Ref< resource::IResourceManager > m_resourceManager;
};
	
	}
}

#endif	// traktor_world_WorldEntityFactory_H
