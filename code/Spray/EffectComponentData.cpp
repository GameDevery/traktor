#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectComponentData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectComponentData", 0, EffectComponentData, world::IEntityComponentData)

Ref< EffectComponent > EffectComponentData::createComponent(resource::IResourceManager* resourceManager, world::EntityEventManager* eventManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< Effect > effect;
	if (!resourceManager->bind(m_effect, effect))
		return nullptr;

	return new EffectComponent(
		effect,
		eventManager,
		soundPlayer
	);
}

void EffectComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< Effect >(L"effect", m_effect);
}

	}
}
