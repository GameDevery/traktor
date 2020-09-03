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
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
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
#include "Scene/Editor/RenderControls/CameraRenderControl.h"
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
#include "World/Entity/CameraComponent.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_defaultMultiSample = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CameraRenderControl", CameraRenderControl, ISceneRenderControl)

CameraRenderControl::CameraRenderControl()
:	m_imageProcessQuality(world::Quality::Disabled)
,	m_shadowQuality(world::Quality::Disabled)
,	m_reflectionsQuality(world::Quality::Disabled)
,	m_motionBlurQuality(world::Quality::Disabled)
,	m_ambientOcclusionQuality(world::Quality::Disabled)
,	m_antiAliasQuality(world::Quality::Disabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool CameraRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT(m_context);

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_multiSample = settings->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect, ui::WsNoCanvas))
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

	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &CameraRenderControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_timer.start();
	return true;
}

void CameraRenderControl::destroy()
{
	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void CameraRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
	Ref< EntityRendererCache > entityRendererCache = new EntityRendererCache(m_context);
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
		{
			Ref< EntityRendererAdapter > entityRenderer = new EntityRendererAdapter(entityRendererCache, *j, [&](const EntityAdapter* adapter) {
				return adapter->isVisible();
			});
			worldEntityRenderers->add(entityRenderer);
		}
	}

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	std::wstring worldRendererTypeName = settings->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");

	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName.c_str());
	if (!worldRendererType)
		return;

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &m_worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.quality.motionBlur = m_motionBlurQuality;
	wcd.quality.reflections = m_reflectionsQuality;
	wcd.quality.shadows = m_shadowQuality;
	wcd.quality.ambientOcclusion = m_ambientOcclusionQuality;
	wcd.quality.antiAlias = m_antiAliasQuality;
	wcd.quality.imageProcess = m_imageProcessQuality;
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

void CameraRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
}

void CameraRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void CameraRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	m_imageProcessQuality = imageProcess;
	m_shadowQuality = shadows;
	m_reflectionsQuality = reflections;
	m_motionBlurQuality = motionBlur;
	m_ambientOcclusionQuality = ambientOcclusion;
	m_antiAliasQuality = antiAlias;
	updateWorldRenderer();
}

void CameraRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
}

void CameraRenderControl::setDebugOverlayAlpha(float alpha)
{
}

bool CameraRenderControl::handleCommand(const ui::Command& command)
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

void CameraRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
}

bool CameraRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool CameraRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

bool CameraRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void CameraRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
}

void CameraRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void CameraRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_colorGrid = settings->getProperty< Color4ub >(L"Editor.Colors/Grid");
	m_colorRef = settings->getProperty< Color4ub >(L"Editor.Colors/ReferenceEdge");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
}

void CameraRenderControl::eventPaint(ui::PaintEvent* event)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance || !m_renderView)
		return;

	// Get current camera entity.
	if (!m_context->findAdaptersOfType(type_of< world::CameraComponent >(), m_cameraEntities))
		return;

	// Check if size has changed since last render; need to reset renderer if so.
	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx != m_dirtySize.cx || sz.cy != m_dirtySize.cy)
	{
		if (!m_renderView->reset(sz.cx, sz.cy))
			return;
		m_dirtySize = sz;
	}

	world::CameraComponent* cameraComponent = m_cameraEntities[0]->getComponent< world::CameraComponent >();
	T_ASSERT(cameraComponent);

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
	Matrix44 projection = m_worldRenderView.getProjection();
	Matrix44 view = m_cameraEntities[0]->getTransform().inverse().toMatrix44();

	// Build a root entity by gathering entities from containers.
	world::GroupEntity rootEntity;
	m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootEntity.addEntity(entity); });
	rootEntity.addEntity(sceneInstance->getRootEntity());

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	if (cameraComponent->getProjection() == world::Projection::Orthographic)
	{
		m_worldRenderView.setOrthogonal(
			cameraComponent->getWidth(),
			cameraComponent->getHeight(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}
	else // Projection::Perspective
	{
		m_worldRenderView.setPerspective(
			float(sz.cx),
			float(sz.cy),
			float(sz.cx) / sz.cy,
			cameraComponent->getFieldOfView(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}
	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(m_worldRenderView, &rootEntity, *m_renderGraph, 0);

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

	event->consume();
}

	}
}
