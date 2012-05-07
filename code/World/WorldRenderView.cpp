#include "World/WorldRenderView.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderView", WorldRenderView, Object)

WorldRenderView::WorldRenderView()
:	m_projection(Matrix44::identity())
,	m_squareProjection(Matrix44::identity())
,	m_view(Matrix44::identity())
,	m_viewSize(0.0f, 0.0f)
,	m_lightCount(0)
,	m_time(0.0f)
,	m_deltaTime(0.0f)
,	m_interval(0.0f)
,	m_interocularDistance(4.5f)
,	m_distortionValue(0.8f)
,	m_screenPlaneDistance(13.0f)
{
	for (int i = 0; i < MaxLightCount; ++i)
	{
		m_lights[i].type = LtDisabled;
		m_lights[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_lights[i].direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		m_lights[i].sunColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
		m_lights[i].baseColor = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
		m_lights[i].shadowColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		m_lights[i].range = Scalar(0.0f);
	}
}

void WorldRenderView::setViewFrustum(const Frustum& viewFrustum)
{
	m_viewFrustum = viewFrustum;
}

void WorldRenderView::setCullFrustum(const Frustum& cullFrustum)
{
	m_cullFrustum = cullFrustum;
}

void WorldRenderView::setProjection(const Matrix44& projection)
{
	m_projection = projection;
}

void WorldRenderView::setSquareProjection(const Matrix44& squareProjection)
{
	m_squareProjection = squareProjection;
}

void WorldRenderView::setView(const Matrix44& view)
{
	m_view = view;
}

void WorldRenderView::setViewSize(const Vector2& viewSize)
{
	m_viewSize = viewSize;
}

void WorldRenderView::setShadowBox(const Aabb3& shadowBox)
{
	m_shadowBox = shadowBox;
}

void WorldRenderView::setTimes(float time, float deltaTime, float interval)
{
	m_time = time;
	m_deltaTime = deltaTime;
	m_interval = interval;
}

void WorldRenderView::setInterocularDistance(float interocularDistance)
{
	m_interocularDistance = interocularDistance;
}

void WorldRenderView::setDistortionValue(float distortionValue)
{
	m_distortionValue = distortionValue;
}

void WorldRenderView::setScreenPlaneDistance(float screenPlaneDistance)
{
	m_screenPlaneDistance = screenPlaneDistance;
}

void WorldRenderView::addLight(const Light& light)
{
	if (m_lightCount < MaxLightCount)
		m_lights[m_lightCount++] = light;
}

void WorldRenderView::resetLights()
{
	m_lightCount = 0;
}

	}
}
