#pragma once

#include <string>
#include "Core/RefArray.h"
#include "World/IEntityComponent.h"

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

class Entity;

/*! Group component.
 * \ingroup World
 */
class T_DLLCLASS GroupComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	GroupComponent();

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void addEntity(Entity* entity);

	void removeEntity(Entity* entity);

	void removeAllEntities();

	const RefArray< Entity >& getEntities() const;

	world::Entity* getEntity(const std::wstring& name, int32_t index) const;

	RefArray< world::Entity > getEntities(const std::wstring& name) const;

private:
	Entity* m_owner;
	Transform m_transform;
	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_deferred[2];
};

	}
}
