#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
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

class IEntityEvent;

class T_DLLCLASS EventSetComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	world::IEntityEvent* getEvent(const std::wstring& name) const;

private:
	friend class EventSetComponentData;

	SmallMap< std::wstring, Ref< IEntityEvent > > m_events;
};

	}
}
