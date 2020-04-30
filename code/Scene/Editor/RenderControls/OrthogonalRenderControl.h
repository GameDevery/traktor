#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Timer/Timer.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Scene/Editor/RenderControlModel.h"
#include "Ui/Rect.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace render
	{

class IRenderView;
class RenderContext;
class RenderGraph;
class PrimitiveRenderer;

	}

	namespace world
	{

class IWorldRenderer;

	}

	namespace scene
	{

class Camera;
class SceneEditorContext;

class OrthogonalRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	enum ViewPlane
	{
		PositiveX = 0,
		NegativeX = 1,
		PositiveY = 2,
		NegativeY = 3,
		PositiveZ = 4,
		NegativeZ = 5
	};

	OrthogonalRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, ViewPlane viewPlane, int32_t cameraId, const TypeInfo& worldRendererType);

	virtual void destroy() override final;

	virtual void updateWorldRenderer() override final;

	virtual void setWorldRendererType(const TypeInfo& worldRendererType) override final;

	virtual void setAspect(float aspect) override final;

	virtual void setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality) override final;

	virtual void setDebugOverlay(world::IDebugOverlay* overlay) override final;
	
	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual bool hitTest(const ui::Point& position) const override final;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const override final;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const override final;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) override final;

	virtual void showSelectionRectangle(const ui::Rect& rect) override final;

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	const TypeInfo* m_worldRendererType;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::Quality m_shadowQuality;
	world::Quality m_reflectionsQuality;
	world::Quality m_motionBlurQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
	RenderControlModel m_model;
	bool m_gridEnable;
	bool m_guideEnable;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Color4ub m_colorRef;
	Color4ub m_colorCamera;
	int32_t m_multiSample;
	Timer m_timer;
	ViewPlane m_viewPlane;
	float m_viewFarZ;
	float m_magnification;
	Ref< Camera > m_camera;
	ui::Rect m_selectionRectangle;
	ui::Size m_dirtySize;
	int32_t m_worldIndex;

	void updateSettings();

	Matrix44 getProjectionTransform() const;

	Matrix44 getViewTransform() const;

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}

