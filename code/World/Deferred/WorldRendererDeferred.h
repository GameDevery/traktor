#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

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

class ImageProcess;
class ISimpleTexture;
class RenderContext;
class RenderTargetSet;

	}

	namespace world
	{

class IWorldShadowProjection;
class LightRendererDeferred;
class WorldContext;

/*! \brief World renderer deferred implementation.
 * \ingroup World
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererDeferred();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual bool beginBuild() override final;

	virtual void build(Entity* entity) override final;

	virtual void endBuild(WorldRenderView& worldRenderView, int frame) override final;

	virtual bool beginRender(int frame, render::EyeType eye, const Color4f& clearColor) override final;

	virtual void render(int frame, render::EyeType eye) override final;

	virtual void endRender(int frame, render::EyeType eye, float deltaTime) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct Slice
	{
		Ref< WorldContext > shadow[MaxLightShadowCount];
		Matrix44 shadowLightView[MaxLightShadowCount];
		Matrix44 shadowLightProjection[MaxLightShadowCount];
		Matrix44 viewToLightSpace[MaxLightShadowCount];
	};

	struct Frame
	{
		Slice slice[MaxSliceCount];
		Ref< WorldContext > gbuffer;
		Ref< WorldContext > irradiance;
		Ref< WorldContext > velocity;
		Ref< WorldContext > visual;
		float time;
		float A;
		float B;
		Matrix44 projection;
		Matrix44 lastView;
		Matrix44 view;
		Frustum viewFrustum;
		AlignedVector< Light > lights;
		Vector4 godRayDirection;
		bool haveGBuffer;
		bool haveIrradiance;
		bool haveVelocity;

		Frame()
		:	time(0.0f)
		,	A(0.0f)
		,	B(0.0f)
		,	haveGBuffer(false)
		,	haveIrradiance(false)
		,	haveVelocity(false)
		{
		}
	};

	static render::handle_t ms_techniqueDeferredColor;
	static render::handle_t ms_techniqueDeferredGBufferWrite;
	static render::handle_t ms_techniqueIrradianceWrite;
	static render::handle_t ms_techniqueVelocityWrite;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleView;
	static render::handle_t ms_handleViewInverse;
	static render::handle_t ms_handleProjection;
	static render::handle_t ms_handleColorMap;
	static render::handle_t ms_handleDepthMap;
	static render::handle_t ms_handleLightMap;
	static render::handle_t ms_handleNormalMap;
	static render::handle_t ms_handleMiscMap;
	static render::handle_t ms_handleReflectionMap;
	static render::handle_t ms_handleFogDistanceAndDensity;
	static render::handle_t ms_handleFogColor;
	static render::handle_t ms_handleShadowMask;

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;
	Quality m_toneMapQuality;
	Quality m_motionBlurQuality;
	Quality m_shadowsQuality;
	Quality m_reflectionsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderView > m_renderView;
	Ref< IWorldShadowProjection > m_shadowProjection0;
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderTargetSet > m_velocityTargetSet;
	Ref< render::RenderTargetSet > m_colorTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskProjectTargetSet;
	Ref< render::RenderTargetSet > m_lightAccumulationTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< render::ImageProcess > m_shadowMaskProject;
	Ref< render::ImageProcess > m_colorTargetCopy;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_motionBlurPrimeImageProcess;
	Ref< render::ImageProcess > m_motionBlurImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;
	Ref< LightRendererDeferred > m_lightRenderer;
	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;
	bool m_includeObjectVelocity;

	void buildGBuffer(WorldRenderView& worldRenderView, int frame);

	void buildIrradiance(WorldRenderView& worldRenderView, int frame);

	void buildVelocity(WorldRenderView& worldRenderView, int frame);

	void buildLightWithShadows(WorldRenderView& worldRenderView, int frame);

	void buildLightWithNoShadows(WorldRenderView& worldRenderView, int frame);

	void buildVisual(WorldRenderView& worldRenderView, int frame);
};

	}
}
