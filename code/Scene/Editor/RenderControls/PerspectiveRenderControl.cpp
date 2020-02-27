#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Format.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Scene/Editor/RenderControls/PerspectiveRenderControl.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/AspectLayout.h"
#include "Ui/Itf/IWidget.h"
#include "World/EntityEventManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const resource::Id< render::Shader > c_debugShader(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));

const float c_defaultFieldOfView = 80.0f;
const float c_defaultMouseWheelRate = 10.0f;
const int32_t c_defaultMultiSample = 0;
const float c_minFieldOfView = 4.0f;
const float c_maxFieldOfView = 160.0f;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

const wchar_t* c_visualizeTechniques[] =
{
	L"Default",
	L"UnitDepth",
	L"ViewDepth",
	L"Normals",
	L"Velocity",
	L"Roughness",
	L"Metalness",
	L"Specular",
	L"LightMask",
	L"ShadowMap",
	L"ShadowMask"
};

Vector4 projectUnit(const ui::Rect& rc, const ui::Point& pnt)
{
	return Vector4(
		2.0f * float(pnt.x - rc.left) / rc.getWidth() - 1.0f,
		1.0f - 2.0f * float(pnt.y - rc.top) / rc.getHeight(),
		0.0f,
		1.0f
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PerspectiveRenderControl", PerspectiveRenderControl, ISceneRenderControl)

PerspectiveRenderControl::PerspectiveRenderControl()
:	m_debugAlpha(1.0f)
,	m_worldRendererType(nullptr)
,	m_imageProcessQuality(world::QuDisabled)
,	m_shadowQuality(world::QuDisabled)
,	m_reflectionsQuality(world::QuDisabled)
,	m_motionBlurQuality(world::QuDisabled)
,	m_ambientOcclusionQuality(world::QuDisabled)
,	m_antiAliasQuality(world::QuDisabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_fieldOfView(c_defaultFieldOfView)
,	m_mouseWheelRate(c_defaultMouseWheelRate)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool PerspectiveRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, const TypeInfo& worldRendererType)
{
	m_context = context;
	T_ASSERT(m_context);

	m_worldRendererType = &worldRendererType;

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);
	m_multiSample = settings->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect, ui::WsWantAllInput))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_multiSample;
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_context->getRenderSystem());

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		1
	))
		return false;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(
		m_context->getRenderSystem()
	))
		return false;

	if (!m_context->getResourceManager()->bind(c_debugShader, m_debugShader))
		return false;

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &PerspectiveRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &PerspectiveRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &PerspectiveRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &PerspectiveRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &PerspectiveRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &PerspectiveRenderControl::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &PerspectiveRenderControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_worldRenderView.setIndex(cameraId);

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(true);
	m_timer.start();

	return true;
}

void PerspectiveRenderControl::destroy()
{
	if (m_camera)
	{
		m_camera->setEnable(false);
		m_camera = nullptr;
	}

	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void PerspectiveRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
	Ref< EntityRendererCache > entityRendererCache = new EntityRendererCache(m_context);
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		editorProfile->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (auto entityRenderer : entityRenderers)
		{
			Ref< EntityRendererAdapter > entityRendererAdapter = new EntityRendererAdapter(entityRendererCache, entityRenderer, [&](const EntityAdapter * adapter) {
				return adapter->isVisible();
			});
			worldEntityRenderers->add(entityRendererAdapter);
		}
	}

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(m_worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &m_worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.motionBlurQuality = m_motionBlurQuality;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.reflectionsQuality = m_reflectionsQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.imageProcessQuality = m_imageProcessQuality;
	wcd.multiSample = m_multiSample;
	wcd.frameCount = 1;

	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		wcd
	))
	{
		m_worldRenderer = worldRenderer;
	}
}

void PerspectiveRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	m_worldRendererType = &worldRendererType;
	updateWorldRenderer();
}

void PerspectiveRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void PerspectiveRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	m_imageProcessQuality = imageProcessQuality;
	m_shadowQuality = shadowQuality;
	m_reflectionsQuality = reflectionsQuality;
	m_motionBlurQuality = motionBlurQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	updateWorldRenderer();
}

bool PerspectiveRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.SettingsChanged")
		updateSettings();
	else if (command == L"Scene.Editor.EnableGrid")
		m_gridEnable = true;
	else if (command == L"Scene.Editor.DisableGrid")
		m_gridEnable = false;
	else if (command == L"Scene.Editor.EnableGuide")
		m_guideEnable = true;
	else if (command == L"Scene.Editor.DisableGuide")
		m_guideEnable = false;

	return result;
}

void PerspectiveRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
}

bool PerspectiveRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	ui::Rect innerRect = m_renderWidget->getInnerRect();

	Scalar fx(float(position.x) / innerRect.getWidth());
	Scalar fy(float(position.y) / innerRect.getHeight());

	// Interpolate frustum edges to find view pick-ray.
	const Vector4& viewEdgeTopLeft = viewFrustum.corners[4];
	const Vector4& viewEdgeTopRight = viewFrustum.corners[5];
	const Vector4& viewEdgeBottomLeft = viewFrustum.corners[7];
	const Vector4& viewEdgeBottomRight = viewFrustum.corners[6];

	Vector4 viewEdgeTop = lerp(viewEdgeTopLeft, viewEdgeTopRight, fx);
	Vector4 viewEdgeBottom = lerp(viewEdgeBottomLeft, viewEdgeBottomRight, fx);
	Vector4 viewRayDirection = lerp(viewEdgeTop, viewEdgeBottom, fy).normalized().xyz0();

	// Transform ray into world space.
	Matrix44 viewInv = m_worldRenderView.getView().inverse();
	outWorldRayOrigin = viewInv.translation().xyz1();
	outWorldRayDirection = viewInv * viewRayDirection;

	return true;
}

bool PerspectiveRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	Vector4 origin[4], direction[4];
	calculateRay(rc.getTopRight(), origin[0], direction[0]);
	calculateRay(rc.getTopLeft(), origin[1], direction[1]);
	calculateRay(rc.getBottomLeft(), origin[2], direction[2]);
	calculateRay(rc.getBottomRight(), origin[3], direction[3]);

	Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	Scalar nz = viewFrustum.getNearZ();
	Scalar fz = viewFrustum.getFarZ();

	Vector4 corners[8] =
	{
		origin[0] + direction[0] * nz,
		origin[0] + direction[1] * nz,
		origin[0] + direction[2] * nz,
		origin[0] + direction[3] * nz,
		origin[0] + direction[0] * fz,
		origin[0] + direction[1] * fz,
		origin[0] + direction[2] * fz,
		origin[0] + direction[3] * fz,
	};

	Plane planes[6] =
	{
		Plane(corners[1], corners[6], corners[5]),
		Plane(corners[3], corners[4], corners[7]),
		Plane(corners[2], corners[7], corners[6]),
		Plane(corners[0], corners[5], corners[4]),
		Plane(corners[0], corners[2], corners[1]),
		Plane(corners[4], corners[6], corners[7])
	};

	outWorldFrustum.buildFromPlanes(planes);
	return true;
}

bool PerspectiveRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void PerspectiveRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	Vector4 delta = mouseDelta;
	switch (mode)
	{
	case McmRotate:
		delta *= Scalar(c_cameraRotateDeltaScale);
		m_camera->rotate(delta.y(), delta.x());
		break;

	case McmMoveXZ:
		delta *= Scalar(c_cameraTranslateDeltaScale);
		m_camera->move(delta.shuffle< 0, 2, 1, 3 >());
		break;

	case McmMoveXY:
		if (!m_invertPanY)
			delta *= Vector4(c_cameraTranslateDeltaScale, -c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		else
			delta *= Vector4(c_cameraTranslateDeltaScale, c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		m_camera->move(delta.shuffle< 0, 1, 2, 3 >());
		break;
	}
}

void PerspectiveRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void PerspectiveRenderControl::getDebugTargets(std::vector< render::DebugTarget >& outDebugTargets)
{
	if (m_renderGraph)
		m_renderGraph->getDebugTargets(outDebugTargets);
}

void PerspectiveRenderControl::setDebugTarget(const render::DebugTarget* debugTarget, float alpha)
{
	if (debugTarget)
		m_debugTarget = *debugTarget;
	else
		m_debugTarget.texture = nullptr;

	m_debugAlpha = alpha;
}

void PerspectiveRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_colorGrid = settings->getProperty< Color4ub >(L"Editor.Colors/Grid");
	m_colorRef = settings->getProperty< Color4ub >(L"Editor.Colors/ReferenceEdge");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);

	updateWorldRenderer();
}

Matrix44 PerspectiveRenderControl::getProjectionTransform() const
{
	return m_worldRenderView.getProjection();
}

Matrix44 PerspectiveRenderControl::getViewTransform() const
{
	return m_camera->getView().toMatrix44();
}

void PerspectiveRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	int32_t rotation = event->getRotation();

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		rotation = -rotation;

	m_camera->move(Vector4(0.0f, 0.0f, rotation * -m_mouseWheelRate, 0.0f));
	m_context->raiseCameraMoved();
	m_context->raiseRedraw();
}

void PerspectiveRenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::Size sz = event->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	m_dirtySize = sz;
}

void PerspectiveRenderControl::eventPaint(ui::PaintEvent* event)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance || !m_renderView || !m_primitiveRenderer)
		return;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	float colorClear[4]; m_colorClear.getRGBA32F(colorClear);
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	ui::Size sz = m_renderWidget->getInnerRect().getSize();

	// Get current transformations.
	Matrix44 projection = getProjectionTransform();
	Matrix44 view = getViewTransform();

	// Build a root entity by gathering entities from containers.
	world::GroupEntity rootEntity;
	m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootEntity.addEntity(entity); });
	rootEntity.addEntity(sceneInstance->getRootEntity());

	// Setup world render passes.
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(m_fieldOfView),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);
	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(m_worldRenderView, &rootEntity, *m_renderGraph, 0);

	// Validate render graph.
	if (!m_renderGraph->validate(m_dirtySize.cx, m_dirtySize.cy))
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext);

	// Render frame.
	render::Clear clear = {};
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(colorClear[0], colorClear[1], colorClear[2], colorClear[3]);
	clear.depth = 1.0f;
	clear.stencil = 0;
	if (m_renderView->begin(&clear))
	{
		// Render context.
		m_renderContext->render(m_renderView);

		// \fixme Render debug target.
		// if (m_debugTarget.texture)
		// {
		// 	m_debugShader->setTechnique(c_visualizeTechniques[m_debugTarget.visualize]);
		// 	m_debugShader->setTextureParameter(L"Scene_DebugTexture", m_debugTarget.texture);
		// 	m_debugShader->setFloatParameter(L"Scene_DebugAlpha", m_debugAlpha);
		// 	m_screenRenderer->draw(m_renderView, m_debugShader);
		// }

		// Render wire guides.
		m_primitiveRenderer->begin(0, projection);
		m_primitiveRenderer->setClipDistance(m_worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushView(view);

		// Render XZ grid.
		if (m_gridEnable)
		{
			Vector4 viewPosition = view.inverse().translation();
			float vx = floorf(viewPosition.x());
			float vz = floorf(viewPosition.z());

			for (int32_t x = -20; x <= 20; ++x)
			{
				float fx = float(x);
				m_primitiveRenderer->drawLine(
					Vector4(fx + vx, 0.0f, -20.0f + vz, 1.0f),
					Vector4(fx + vx, 0.0f, 20.0f + vz, 1.0f),
					(int(fx + vx) == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
				m_primitiveRenderer->drawLine(
					Vector4(-20.0f + vx, 0.0f, fx + vz, 1.0f),
					Vector4(20.0f + vx, 0.0f, fx + vz, 1.0f),
					(int(fx + vz) == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			// Draw frame.
			const float c_arrowLength = 0.4f;
			const float c_frameSize = 0.2f;

			float w = 2.0f * float(sz.cx) / sz.cy;
			float h = 2.0f;

			m_primitiveRenderer->setProjection(orthoLh(-w / 2.0f, -h / 2.0f, w / 2.0f, h / 2.0f, -1.0f, 1.0f));
			m_primitiveRenderer->pushWorld(Matrix44::identity());
			m_primitiveRenderer->pushView(
				translate(w / 2.0f - c_frameSize, h / 2.0f - c_frameSize, 0.0f) *
				scale(c_frameSize, c_frameSize, c_frameSize)
			);

			m_primitiveRenderer->pushDepthState(false, true, false);
			m_primitiveRenderer->drawSolidQuad(
				Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				Vector4(1.0f, -1.0f, 1.0f, 1.0f),
				Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
				Color4ub(0, 0, 0, 32)
			);
			m_primitiveRenderer->popDepthState();

			m_primitiveRenderer->pushDepthState(true, true, false);

			m_primitiveRenderer->drawLine(
				Vector4::origo(),
				Vector4::origo() + view.axisX() * Scalar(1.0f - c_arrowLength),
				Color4ub(255, 0, 0, 255)
			);
			m_primitiveRenderer->drawArrowHead(
				Vector4::origo() + view.axisX() * Scalar(1.0f - c_arrowLength),
				Vector4::origo() + view.axisX(),
				0.8f,
				Color4ub(255, 0, 0, 255)
			);

			m_primitiveRenderer->drawLine(
				Vector4::origo(),
				Vector4::origo() + view.axisY() * Scalar(1.0f - c_arrowLength),
				Color4ub(0, 255, 0, 255)
			);
			m_primitiveRenderer->drawArrowHead(
				Vector4::origo() + view.axisY() * Scalar(1.0f - c_arrowLength),
				Vector4::origo() + view.axisY(),
				0.8f,
				Color4ub(0, 255, 0, 255)
			);

			m_primitiveRenderer->drawLine(
				Vector4::origo(),
				Vector4::origo() + view.axisZ() * Scalar(1.0f - c_arrowLength),
				Color4ub(0, 0, 255, 255)
			);
			m_primitiveRenderer->drawArrowHead(
				Vector4::origo() + view.axisZ() * Scalar(1.0f - c_arrowLength),
				Vector4::origo() + view.axisZ(),
				0.8f,
				Color4ub(0, 0, 255, 255)
			);

			m_primitiveRenderer->popWorld();
			m_primitiveRenderer->popView();
			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->setProjection(projection);
		}

		// Draw guides.
		if (m_guideEnable)
		{
			RefArray< EntityAdapter > entityAdapters;
			m_context->getEntities(entityAdapters, SceneEditorContext::GfDefault);
			for (auto entityAdapter : entityAdapters)
				entityAdapter->drawGuides(m_primitiveRenderer);

			// Draw controller guides.
			ISceneControllerEditor* controllerEditor = m_context->getControllerEditor();
			if (controllerEditor)
				controllerEditor->draw(m_primitiveRenderer);
		}

		// Draw modifier.
		IModifier* modifier = m_context->getModifier();
		if (modifier)
			modifier->draw(m_primitiveRenderer);

		// Draw selection rectangle if non-empty.
		if (m_selectionRectangle.area() > 0)
		{
			ui::Rect innerRect = m_renderWidget->getInnerRect();

			m_primitiveRenderer->setProjection(orthoLh(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f));

			m_primitiveRenderer->pushView(Matrix44::identity());
			m_primitiveRenderer->pushDepthState(false, false, false);

			m_primitiveRenderer->drawSolidQuad(
				projectUnit(innerRect, m_selectionRectangle.getTopLeft()),
				projectUnit(innerRect, m_selectionRectangle.getTopRight()),
				projectUnit(innerRect, m_selectionRectangle.getBottomRight()),
				projectUnit(innerRect, m_selectionRectangle.getBottomLeft()),
				Color4ub(0, 64, 128, 128)
			);
			m_primitiveRenderer->drawWireQuad(
				projectUnit(innerRect, m_selectionRectangle.getTopLeft()),
				projectUnit(innerRect, m_selectionRectangle.getTopRight()),
				projectUnit(innerRect, m_selectionRectangle.getBottomRight()),
				projectUnit(innerRect, m_selectionRectangle.getBottomLeft()),
				Color4ub(120, 190, 250, 255)
			);

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->popView();
		}

		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
