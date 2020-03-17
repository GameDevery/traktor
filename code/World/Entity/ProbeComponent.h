#pragma once

#include "Resource/Proxy.h"
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
	namespace render
	{

class ICubeTexture;

	}

	namespace world
	{

/*! Reflection probe component.
 * \ingroup World
 */
class T_DLLCLASS ProbeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ProbeComponent(
		const resource::Proxy< render::ICubeTexture >& texture,
		float intensity,
		bool local,
		const Aabb3& volume,
		bool dirty
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	void setTexture(const resource::Proxy< render::ICubeTexture >& texture) { m_texture = texture; }

	const resource::Proxy< render::ICubeTexture >& getTexture() const { return m_texture; }

	float getIntensity() const { return m_intensity; }

	bool getLocal() const { return m_local; }

	const Aabb3& getVolume() const { return m_volume; }

	void setDirty(bool dirty) { m_dirty = dirty; }

	bool getDirty() const { return m_dirty; }

private:
	Entity* m_owner;
	resource::Proxy< render::ICubeTexture > m_texture;
	float m_intensity;
	bool m_local;
	Aabb3 m_volume;
	bool m_dirty;
};

	}
}
