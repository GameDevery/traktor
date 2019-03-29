#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;

class T_DLLCLASS RenderControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	RenderControl();

	bool create(ui::Widget* parent, IRenderSystem* renderSystem, db::Database* database);

	virtual void destroy();

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< IRenderView > m_renderView;
	Ref< PrimitiveRenderer > m_primitiveRenderer;
	float m_cameraHead;
	float m_cameraPitch;
	float m_cameraZ;
	ui::Point m_lastMousePosition;

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventSize(ui::SizeEvent* event);
};

	}
}

