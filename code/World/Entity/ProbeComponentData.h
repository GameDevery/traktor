#pragma once

#include "Core/Math/Aabb3.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"
//#include "World/WorldTypes.h"

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
class T_DLLCLASS ProbeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ProbeComponentData();

	virtual void serialize(ISerializer& s) override final;

	void setTexture(const resource::Id< render::ICubeTexture >& texture) { m_texture = texture; }

	const resource::Id< render::ICubeTexture >& getTexture() const { return m_texture; }

	void setIntensity(float intensity) { m_intensity = intensity; }

	float getIntensity() const { return m_intensity; }

	void setLocal(bool local) { m_local = local; }

	bool getLocal() const { return m_local; }

	void setVolume(const Aabb3& volume) { m_volume = volume; }

	const Aabb3& getVolume() const { return m_volume; }

private:
	resource::Id< render::ICubeTexture > m_texture;
	float m_intensity;
	bool m_local;
	Aabb3 m_volume;
};

	}
}
