#include <limits>
#include "Core/Math/Log2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/RenderControls/OrthogonalRenderControl.h"
#include "Ui/Command.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IWidget.h"
#include "World/Entity.h"
#include "World/EntityEventManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_defaultMultiSample = 0;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_minMagnification = 0.01f;

int32_t translateMouseButton(int32_t uimb)
{
	if (uimb == ui::MbtLeft)
		return 0;
	else if (uimb == ui::MbtRight)
		return 1;
	else if (uimb == ui::MbtMiddle)
		return 2;
	else if (uimb == (ui::MbtLeft | ui::MbtRight))
		return 2;
	else
		return 0;
}

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OrthogonalRenderControl", OrthogonalRenderControl, ISceneRenderControl)

OrthogonalRenderControl::OrthogonalRenderControl()
:	m_worldRendererType(nullptr)
,	m_shadowQuality(world::Quality::Disabled)
,	m_reflectionsQuality(world::Quality::Disabled)
,	m_motionBlurQuality(world::Quality::Disabled)
,	m_ambientOcclusionQuality(world::Quality::Disabled)
,	m_antiAliasQuality(world::Quality::Disabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_multiSample(0)
,	m_viewPlane(PositiveX)
,	m_viewFarZ(0.0f)
,	m_magnification(10.0f)
,	m_dirtySize(0, 0)
,	m_worldIndex(0)
{
}

bool OrthogonalRenderControl::create(ui::Widget* parent, SceneEditorContext* context, ViewPlane viewPlane, int32_t cameraId, const TypeInfo& worldRendererType)
{
	m_context = context;
	T_ASSERT(m_context);

	m_worldRendererType = &worldRendererType;

	m_multiSample = m_context->getEditor()->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);
	m_viewPlane = viewPlane;

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent, ui::WsNoCanvas))
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

	m_renderContext = new render::RenderContext(16 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_context->getRenderSystem());

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		1
	))
		return false;

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &OrthogonalRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &OrthogonalRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &OrthogonalRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &OrthogonalRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &OrthogonalRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &OrthogonalRenderControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_worldIndex = cameraId;

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(false);

	switch (m_viewPlane)
	{
	case PositiveX:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(90.0f))));
		break;
	case NegativeX:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(-90.0f))));
		break;
	case PositiveY:
		m_camera->setOrientation(Quaternion(rotateX(deg2rad(90.0f))));
		break;
	case NegativeY:
		m_camera->setOrientation(Quaternion(rotateX(deg2rad(-90.0f))));
		break;
	case PositiveZ:
		m_camera->setOrientation(Quaternion::identity());
		break;
	case NegativeZ:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(180.0f))));
		break;
	}

	m_timer.start();
	return true;
}

void OrthogonalRenderControl::destroy()
{
	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	safeDestroy(m_renderWidget);
}

void OrthogonalRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	Ref< const world::WorldRenderSettings > worldRenderSettings = sceneInstance->getWorldRenderSettings();

	// Create entity renderers; every renderer is wrapped in a custom renderer in order to check flags etc.
	Ref< EntityRendererCache > entityRendererCache = new EntityRendererCache(m_context);
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		editorProfile->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (auto entityRenderer : entityRenderers)
		{
			Ref< EntityRendererAdapter > entityRendererAdapter = new EntityRendererAdapter(entityRendererCache, entityRenderer, [&](const EntityAdapter* adapter) {
				return adapter->isVisible();
			});
			worldEntityRenderers->add(entityRendererAdapter);
		}
	}

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast<world::IWorldRenderer*>(m_worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	// Create world renderer.
	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.quality.motionBlur = m_motionBlurQuality;
	wcd.quality.shadows = m_shadowQuality;
	wcd.quality.reflections = m_reflectionsQuality;
	wcd.quality.ambientOcclusion = m_ambientOcclusionQuality;
	wcd.quality.antiAlias = m_antiAliasQuality;
	wcd.multiSample = m_multiSample;
	wcd.frameCount = 1;

	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		wcd
	))
		m_worldRenderer = worldRenderer;

	m_viewFarZ = worldRenderSettings->viewFarZ;
}

void OrthogonalRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	m_worldRendererType = &worldRendererType;
	updateWorldRenderer();
}

void OrthogonalRenderControl::setAspect(float aspect)
{
}

void OrthogonalRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	m_shadowQuality = shadows;
	m_reflectionsQuality = reflections;
	m_motionBlurQuality = motionBlur;
	m_ambientOcclusionQuality = ambientOcclusion;
	m_antiAliasQuality = antiAlias;
	updateWorldRenderer();
}

void OrthogonalRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
}

void OrthogonalRenderControl::setDebugOverlayAlpha(float alpha)
{
}

bool OrthogonalRenderControl::handleCommand(const ui::Command& command)
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

void OrthogonalRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
}

bool OrthogonalRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

bool OrthogonalRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	const float c_viewFarOffset = 1.0f;
	const ui::Rect innerRect = m_renderWidget->getInnerRect();

	Matrix44 projection = getProjectionTransform();
	Matrix44 projectionInverse = projection.inverse();

	Matrix44 view = getViewTransform();
	Matrix44 viewInverse = view.inverse();

	Scalar fx( float(position.x * 2.0f) / innerRect.getWidth() - 1.0f);
	Scalar fy(-float(position.y * 2.0f) / innerRect.getHeight() + 1.0f);

	Vector4 clipPosition(fx, fy, 0.0f, 1.0f);
	Vector4 viewPosition = projectionInverse * clipPosition + Vector4(0.0f, 0.0f, -(m_viewFarZ - c_viewFarOffset));
	outWorldRayOrigin = viewInverse * viewPosition;
	outWorldRayDirection = viewInverse.axisZ();

	return true;
}

bool OrthogonalRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	Vector4 origin[4], direction[4];
	calculateRay(rc.getTopRight(), origin[0], direction[0]);
	calculateRay(rc.getTopLeft(), origin[1], direction[1]);
	calculateRay(rc.getBottomLeft(), origin[2], direction[2]);
	calculateRay(rc.getBottomRight(), origin[3], direction[3]);

	Scalar nz(-1e6f);
	Scalar fz(1e6f);

	Vector4 corners[8] =
	{
		origin[0] + direction[0] * nz,
		origin[1] + direction[1] * nz,
		origin[2] + direction[2] * nz,
		origin[3] + direction[3] * nz,
		origin[0] + direction[0] * fz,
		origin[1] + direction[1] * fz,
		origin[2] + direction[2] * fz,
		origin[3] + direction[3] * fz,
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

void OrthogonalRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	if (mode == McmMoveXY || mode == McmMoveXZ)
	{
		Vector4 cameraPosition = m_camera->getPosition();
		cameraPosition -= m_camera->getOrientation() * viewDelta.xyz0();
		m_camera->setPosition(cameraPosition);
	}
}

void OrthogonalRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void OrthogonalRenderControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
	m_colorGrid = colors->getProperty< Color4ub >(L"Grid");
	m_colorRef = colors->getProperty< Color4ub >(L"ReferenceEdge");
	m_colorCamera = colors->getProperty< Color4ub >(L"CameraWire");
}

Matrix44 OrthogonalRenderControl::getProjectionTransform() const
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (m_worldRenderer && sceneInstance)
	{
		const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();

		ui::Rect innerRect = m_renderWidget->getInnerRect();
		float ratio = float(innerRect.getWidth()) / innerRect.getHeight();

		world::WorldRenderView worldRenderView;
		worldRenderView.setOrthogonal(
			m_magnification,
			m_magnification / ratio,
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
		return worldRenderView.getProjection();
	}
	else
		return Matrix44::identity();
}

Matrix44 OrthogonalRenderControl::getViewTransform() const
{
	return m_camera->getView().toMatrix44();
}

void OrthogonalRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	int rotation = event->getRotation();

	float delta = m_magnification / 10.0f;

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		m_magnification -= rotation * delta;
	else
		m_magnification += rotation * delta;

	if (m_magnification < c_minMagnification)
		m_magnification = c_minMagnification;

	m_context->raiseCameraMoved();
	m_context->raiseRedraw();
}

void OrthogonalRenderControl::eventPaint(ui::PaintEvent* event)
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

	// Check if size has changed since last render; need to reset renderer if so.
	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx != m_dirtySize.cx || sz.cy != m_dirtySize.cy)
	{
		if (!m_renderView->reset(sz.cx, sz.cy))
			return;
		m_dirtySize = sz;
	}

	float colorClear[4]; m_colorClear.getRGBA32F(colorClear);
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();
	float ratio = float(m_dirtySize.cx) / m_dirtySize.cy;
	float width = m_magnification;
	float height = m_magnification / ratio;
	Matrix44 view = getViewTransform();

	// Build a root entity by gathering entities from containers.
	Ref< world::GroupComponent > rootGroup = new world::GroupComponent();
	Ref< world::Entity > rootEntity = new world::Entity();
	rootEntity->setComponent(rootGroup);

	m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootGroup->addEntity(entity); });
	rootGroup->addEntity(sceneInstance->getRootEntity());

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	world::WorldRenderView worldRenderView;
	worldRenderView.setIndex(m_worldIndex);
	worldRenderView.setOrthogonal(
		width,
		height,
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);
	worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	worldRenderView.setView(view, view);
	m_worldRenderer->setup(worldRenderView, rootEntity, *m_renderGraph, 0);

	// Draw debug wires.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug");
	rp->setOutput(0, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {

		// Draw wire guides.
		m_primitiveRenderer->begin(0, worldRenderView.getProjection());
		m_primitiveRenderer->setClipDistance(worldRenderView.getViewFrustum().getNearZ());

		// Render grid.
		if (m_gridEnable)
		{
			Vector4 cameraPosition = -(m_camera->getOrientation().inverse() * m_camera->getPosition().xyz1());

			float hx = width * 0.5f;
			float hy = height * 0.5f;

			Vector2 cp(cameraPosition.x(), cameraPosition.y());
			Vector2 vtl(-hx, -hy), vbr(hx, hy);
			Vector2 tl = vtl - cp, br = vbr - cp;

			float lx = floorf(tl.x);
			float rx = ceilf(br.x);
			float ty = floorf(tl.y);
			float by = ceilf(br.y);

			float step = float(
				nearestLog2(int32_t(
					std::max(
						(rx - lx) / 10.0f,
						(by - ty) / 10.0f
					)
				) - 1)
			);

			if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
			{
				float spacing = m_context->getSnapSpacing();
				step = std::max(spacing, step);
			}
			else
				step = std::max(1.0f, step);

			lx -= sign(lx) * std::fmod(abs(lx), step);
			ty -= sign(ty) * std::fmod(abs(ty), step);

			m_primitiveRenderer->pushDepthState(false, false, false);

			for (float x = lx; x <= rx; x += step)
			{
				float fx = x + cameraPosition.x();

				m_primitiveRenderer->drawLine(
					Vector4(fx, vtl.y, 0.0f, 1.0f),
					Vector4(fx, vbr.y, 0.0f, 1.0f),
					(abs(x) < 0.01f) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			for (float y = ty; y <= by; y += step)
			{
				float fy = y + cameraPosition.y();

				m_primitiveRenderer->drawLine(
					Vector4(vtl.x, fy, 1.0f),
					Vector4(vbr.x, fy, 1.0f),
					(abs(y) < 0.01f) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			m_primitiveRenderer->popDepthState();
		}

		m_primitiveRenderer->pushView(view);

		// Draw cameras.
		for (int32_t i = 0; i < 4; ++i)
		{
			const Camera* camera = m_context->getCamera(i);
			if (!camera || !camera->isEnable())
				continue;

			m_primitiveRenderer->pushView(view);
			m_primitiveRenderer->pushWorld(camera->getWorld().toMatrix44());
			m_primitiveRenderer->pushDepthState(false, false, false);

			m_primitiveRenderer->drawWireAabb(
				Vector4::origo(),
				Vector4(0.1f, 0.1f, 0.1f, 0.0f),
				m_colorCamera
			);

			for (int32_t j = 0; j < sizeof_array(c_cameraMeshIndices); j += 2)
			{
				int32_t i1 = c_cameraMeshIndices[j + 0] - 1;
				int32_t i2 = c_cameraMeshIndices[j + 1] - 1;

				const float* v1 = &c_cameraMeshVertices[i1 * 3];
				const float* v2 = &c_cameraMeshVertices[i2 * 3];

				m_primitiveRenderer->drawLine(
					Vector4(v1[0], v1[1], v1[2], 1.0f),
					Vector4(v2[0], v2[1], v2[2], 1.0f),
					m_colorCamera
				);
			}

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->popWorld();
			m_primitiveRenderer->popView();
		}

		// Draw guides.
		if (m_guideEnable)
		{
			RefArray< EntityAdapter > entityAdapters;
			m_context->getEntities(entityAdapters, SceneEditorContext::GfDefault);
			for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
				(*i)->drawGuides(m_primitiveRenderer);

			// Draw controller guides.
			Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
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

		auto rb = renderContext->alloc< render::LambdaRenderBlock >();
		rb->lambda = [&](render::IRenderView* renderView) {
			m_primitiveRenderer->render(m_renderView, 0);
		};
		renderContext->enqueue(rb);
	});

	m_renderGraph->addPass(rp);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, m_dirtySize.cx, m_dirtySize.cy);

	// Render frame.
	if (m_renderView->beginFrame())
	{
		m_renderContext->render(m_renderView);
		m_renderView->endFrame();
		m_renderView->present();
	}

	// Need to clear all entities from our root group since when our root entity
	// goes out of scope it's automatically destroyed.
	rootGroup->removeAllEntities();

	event->consume();
}

	}
}
