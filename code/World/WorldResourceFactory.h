#pragma once

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace world
	{

class IEntityBuilder;

/*! World resource factory.
 * \ingroup World
 */
class T_DLLCLASS WorldResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	WorldResourceFactory(render::IRenderSystem* renderSystem, const IEntityBuilder* entityBuilder);

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< const IEntityBuilder > m_entityBuilder;
};

	}
}

