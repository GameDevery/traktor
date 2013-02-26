#ifndef traktor_weather_CloudEntity_H
#define traktor_weather_CloudEntity_H

#include "Core/RefArray.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Weather/Clouds/CloudParticleCluster.h"
#include "Weather/Clouds/CloudParticleData.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ITexture;
class RenderContext;
class PrimitiveRenderer;
class RenderTargetSet;
class VertexBuffer;
class IndexBuffer;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldRenderView;

	}

	namespace weather
	{

class CloudMask;

class T_DLLCLASS CloudEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	CloudEntity();

	bool create(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& particleShader,
		const resource::Proxy< render::ITexture >& particleTexture,
		const resource::Proxy< render::Shader >& impostorShader,
		const resource::Proxy< CloudMask >& mask,
		uint32_t impostorTargetResolution,
		uint32_t impostorSliceCount,
		uint32_t updateFrequency,
		float updatePositionThreshold,
		float updateDirectionThreshold,
		const CloudParticleData& particleData
	);

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		render::PrimitiveRenderer* primitiveRenderer
	);

	virtual void update(const world::UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

private:
	resource::Proxy< render::Shader > m_particleShader;
	resource::Proxy< render::ITexture > m_particleTexture;
	resource::Proxy< render::Shader > m_impostorShader;
	resource::Proxy< CloudMask > m_mask;
	RefArray< render::RenderTargetSet > m_impostorTargets;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::handle_t m_handleBillboardView;
	render::handle_t m_handleImpostorTarget;
	uint32_t m_impostorSliceCount;
	uint32_t m_updateFrequency;
	float m_updatePositionThreshold;
	float m_updateDirectionThreshold;
	CloudParticleCluster m_cluster;
	CloudParticleData m_particleData;
	Transform m_transform;
	Vector4 m_lastCameraPosition;
	Vector4 m_lastCameraDirection;
	float m_timeUntilUpdate;
	uint32_t m_updateCount;

	void renderCluster(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		render::PrimitiveRenderer* primitiveRenderer,
		const CloudParticleCluster& cluster
	);
};

	}
}

#endif	// traktor_weather_CloudEntity_H
