#include "Animation/PathEntity/PathEntity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntity", PathEntity, world::Entity)

PathEntity::PathEntity(const Transform& transform, const TransformPath& path, TimeMode timeMode, float timeOffset, world::Entity* entity)
:	m_transform(transform)
,	m_path(path)
,	m_timeMode(timeMode)
,	m_entity(entity)
,	m_timeScale(1.0f)
,	m_timeDeltaSign(1.0f)
,	m_time(timeOffset)
{
}

PathEntity::~PathEntity()
{
}

void PathEntity::destroy()
{
	if (m_entity)
	{
		m_entity->destroy();
		m_entity = 0;
	}
}

void PathEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Transform PathEntity::getTransform() const
{
	return m_transform;
}

Aabb3 PathEntity::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb3();
}

void PathEntity::update(const world::UpdateParams& update)
{
	if (!m_entity)
		return;

	if (m_timeMode != TmManual)
	{
		m_time += update.deltaTime * m_timeDeltaSign;

		switch (m_timeMode)
		{
		case TmOnce:
			if (m_time > m_path.getEndTime())
			{
				if (m_listener && abs(m_timeDeltaSign) > 0.0f)
					m_listener->notifyPathFinished(this);

				m_time = m_path.getEndTime();
				m_timeDeltaSign = 0.0f;
			}
			break;

		case TmLoop:
			if (m_time > m_path.getEndTime())
			{
				if (m_listener)
					m_listener->notifyPathFinished(this);

				m_time = 0.0f;
			}
			break;

		case TmPingPong:
			if (m_time > m_path.getEndTime())
			{
				if (m_listener)
					m_listener->notifyPathFinished(this);

				m_time = m_path.getEndTime();
				m_timeDeltaSign = -1.0f;
			}
			else if (m_time < 0.0f)
			{
				if (m_listener)
					m_listener->notifyPathFinished(this);

				m_time = 0.0f;
				m_timeDeltaSign = 1.0f;
			}
			break;

		default:
			break;
		}
	}

	Transform transform = m_path.evaluate(m_time, m_timeMode == TmLoop).transform();

	m_entity->setTransform(m_transform * transform);
	m_entity->update(update);
}

	}
}
