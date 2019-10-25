#pragma once

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IBitmap;
class ToolBarItem;
class ToolTip;
class ToolTipEvent;

/*! Tool bar control.
 * \ingroup UI
 */
class T_DLLCLASS ToolBar : public Widget
{
	T_RTTI_CLASS;

public:
	ToolBar();

	bool create(Widget* parent, int32_t style = WsNone);

	virtual void destroy() override;

	uint32_t addImage(IBitmap* image, uint32_t imageCount);

	uint32_t addItem(ToolBarItem* item);

	void setItem(uint32_t id, ToolBarItem* item);

	Ref< ToolBarItem > getItem(uint32_t id);

	Ref< ToolBarItem > getItem(const Point& at);

	virtual Size getPreferedSize() const override;

private:
	Ref< ToolTip > m_toolTip;
	int32_t m_style;
	Ref< IBitmap > m_imageEnabled;
	Ref< IBitmap > m_imageDisabled;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	RefArray< ToolBarItem > m_items;
	Ref< ToolBarItem > m_trackItem;
	int32_t m_offsetX;

	void clampOffset();

	void eventMouseTrack(MouseTrackEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventShowTip(ToolTipEvent* event);
};

	}
}

