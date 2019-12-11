#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

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

class Shader;

	}

	namespace world
	{

/*! Decal entity data.
 * \ingroup World
 */
class T_DLLCLASS DecalComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	DecalComponentData();

	virtual void serialize(ISerializer& s) override final;

	float getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	float m_size;
	float m_thickness;
	float m_alpha;
	float m_cullDistance;
	resource::Id< render::Shader > m_shader;
};

	}
}

