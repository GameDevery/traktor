#include <ctime>
#include "Render/Shader.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/Emitter.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_maxDeltaTime = 1.0f / 30.0f;
const uint32_t c_updateDenom = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntity", EffectEntity, world::Entity)

EffectEntity::EffectEntity(const Transform& transform, const resource::Proxy< Effect >& effect, sound::SoundSystem* soundSystem, sound::SurroundEnvironment* surroundEnvironment)
:	m_transform(transform)
,	m_effect(effect)
,	m_counter(0)
,	m_enable(true)
{
	m_context.deltaTime = 0.0f;
#if !defined(WINCE)
	m_context.random = RandomGeometry(uint32_t(clock()));
#endif
	m_context.soundSystem = soundSystem;
	m_context.surroundEnvironment = surroundEnvironment;
}

void EffectEntity::render(const Plane& cameraPlane, PointRenderer* pointRenderer)
{
	if (m_effectInstance)
		m_effectInstance->render(pointRenderer, cameraPlane);
}

void EffectEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool EffectEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 EffectEntity::getBoundingBox() const
{
	if (!m_effectInstance)
		return Aabb3();

	const Aabb3& worldBoundingBox = m_effectInstance->getBoundingBox();
	if (worldBoundingBox.empty())
		return Aabb3();

	return worldBoundingBox.transform(m_transform.inverse());
}

Aabb3 EffectEntity::getWorldBoundingBox() const
{
	return m_effectInstance ? m_effectInstance->getBoundingBox() : Aabb3();
}

void EffectEntity::update(const UpdateParams& update)
{
	if ((m_counter++ % c_updateDenom) != 0)
		return;

	if (m_effect.changed() || !m_effectInstance)
	{
		m_effectInstance = m_effect->createInstance();
		if (m_effectInstance)
		{
			// Collect set of techniques used by this effect; we store
			// these in a local set as we want to check against rendering
			// as fast as possible without going through every layer each time.
			m_techniques.clear();

			const RefArray< EffectLayer >& layers = m_effect->getLayers();
			for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
			{
				const Emitter* emitter = (*i)->getEmitter();
				if (!emitter)
					continue;

				const resource::Proxy< render::Shader >& shader = emitter->getShader();
				if (shader)
					shader->getTechniques(m_techniques);
			}
		}
		m_effect.consume();
	}

	if (m_effectInstance)
	{
		m_context.deltaTime = update.deltaTime;
		m_context.deltaTime = min(m_context.deltaTime, c_maxDeltaTime);
		m_context.deltaTime *= c_updateDenom;

		m_effectInstance->update(m_context, m_transform, m_enable);
	}
}

	}
}
