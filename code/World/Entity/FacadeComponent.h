#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
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

/*! Facade component.
 * \ingroup World
 */
class T_DLLCLASS FacadeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	FacadeComponent();

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

    void addEntity(const std::wstring& id, Entity* entity);

    void removeEntity(const std::wstring& id);

    bool show(const std::wstring& id);

	bool showOnly(const std::wstring& id);

    bool hide(const std::wstring& id);

    void hideAll();

    bool isVisible(const std::wstring& id);

	const SmallMap< std::wstring, Ref< Entity > >& getEntities() const { return m_entities; }

    const SmallSet< Entity* >& getVisibleEntities() const { return m_visibleEntities; }

private:
	Entity* m_owner;
	Transform m_transform;
    SmallMap< std::wstring, Ref< Entity > > m_entities;
    SmallSet< Entity* > m_visibleEntities;
};

	}
}
