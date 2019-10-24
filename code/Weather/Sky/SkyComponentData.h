#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class ITexture;
class Shader;

	}

	namespace weather
	{

class SkyComponent;

class T_DLLCLASS SkyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SkyComponentData();

	Ref< SkyComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

private:
	resource::Id< render::Shader > m_shader;
	resource::Id< render::ITexture > m_texture;
	float m_offset;
};

	}
}

