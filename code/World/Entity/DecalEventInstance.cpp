#include "Core/Misc/SafeDestroy.h"
#include "World/IWorldRenderer.h"
#include "World/Entity.h"
#include "World/Entity/DecalComponent.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEventInstance", DecalEventInstance, IEntityEventInstance)

DecalEventInstance::DecalEventInstance(const DecalEvent* event, const Transform& Toffset)
{
	m_entity = new Entity();
	m_entity->setComponent(new DecalComponent(
		event->getSize(),
		event->getThickness(),
		event->getAlpha(),
		event->getCullDistance(),
		event->getShader()
	));
	m_entity->setTransform(Toffset);
}

bool DecalEventInstance::update(const UpdateParams& update)
{
	if (m_entity)
	{
		m_entity->update(update);
		if (m_entity->getComponent< DecalComponent >()->getAlpha() > FUZZY_EPSILON)
			return true;
	}
	return false;
}

void DecalEventInstance::gather(const std::function< void(Entity*) >& fn) const
{
	fn(m_entity);
}

void DecalEventInstance::cancel(Cancel when)
{
	safeDestroy(m_entity);
}

	}
}
