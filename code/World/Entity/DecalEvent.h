#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace world
	{

/*! \brief
 * \ingroup World
 */
class DecalEvent : public IEntityEvent
{
	T_RTTI_CLASS;

public:
	DecalEvent();

	virtual Ref< IEntityEventInstance > createInstance(IEntityEventManager* eventManager, Entity* sender, const Transform& Toffset) const override final;

	float getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	friend class WorldEntityFactory;

	float m_size;
	float m_thickness;
	float m_alpha;
	float m_cullDistance;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

