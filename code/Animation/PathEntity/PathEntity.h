#pragma once

#include "Core/Ref.h"
#include "Core/Math/TransformPath.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

/*! Movement path entity.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	enum TimeMode
	{
		TmManual,
		TmOnce,
		TmLoop,
		TmPingPong
	};

	struct IListener : public IRefCount
	{
		virtual void notifyPathFinished(PathEntity* entity) = 0;
	};

	PathEntity(const Transform& transform, const TransformPath& path, TimeMode timeMode, float timeOffset, world::Entity* entity);

	virtual ~PathEntity();

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Transform getTransform() const override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	void setPath(const TransformPath& path) { m_path = path; }

	const TransformPath& getPath() const { return m_path; }

	void setTimeMode(TimeMode timeMode) { m_timeMode = timeMode; }

	TimeMode getTimeMode() const { return m_timeMode; }

	void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	float getTimeScale() const { return m_timeScale; }

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	world::Entity* getEntity() { return m_entity; }

	void setListener(IListener* listener) { m_listener = listener; }

private:
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	Ref< world::Entity > m_entity;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
	Ref< IListener > m_listener;
};

	}
}

