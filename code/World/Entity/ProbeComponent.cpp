#include "World/Entity.h"
#include "World/Entity/ProbeComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeComponent", ProbeComponent, IEntityComponent)

ProbeComponent::ProbeComponent(
	const resource::Proxy< render::ICubeTexture >& texture,
	float intensity,
	const Aabb3& volume,
	bool local,
	bool capture,
	bool dirty
)
:	m_owner(nullptr)
,	m_texture(texture)
,	m_intensity(intensity)
,	m_volume(volume)
,	m_local(local)
,	m_capture(capture)
,	m_dirty(dirty)
{
}

void ProbeComponent::destroy()
{
	m_owner = nullptr;
}

void ProbeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
	m_dirty = true;
}

void ProbeComponent::update(const UpdateParams& update)
{
}

void ProbeComponent::setTransform(const Transform& transform)
{
	m_dirty = true;
}

Aabb3 ProbeComponent::getBoundingBox() const
{
	return m_volume;
}

Transform ProbeComponent::getTransform() const
{
	return m_owner->getTransform();
}

	}
}
