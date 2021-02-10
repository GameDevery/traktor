#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/FloodLayout.h"
#include "Ui/Form.h"
#include "Ui/Image.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolForm.h"

// Resources
#include "Resources/DockBottom.h"
#include "Resources/DockLeft.h"
#include "Resources/DockRight.h"
#include "Resources/DockTop.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_hintSize = 32 + 29 + 32;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Dock", Dock, Widget)

bool Dock::create(Widget* parent)
{
	if (!ui::Widget::create(parent, WsAccelerated))
		return false;

	m_hint = new ToolForm();
	m_hint->create(this, L"Hint", 0, 0, WsNone);
	m_hint->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	const int xy[] = { 0, 32, 32 + 29, 32 + 29 + 32 };
	Point p[] =
	{
		Point(xy[1], xy[0]),
		Point(xy[2], xy[0]),
		Point(xy[2], xy[1]),
		Point(xy[3], xy[1]),
		Point(xy[3], xy[2]),
		Point(xy[2], xy[2]),
		Point(xy[2], xy[3]),
		Point(xy[1], xy[3]),
		Point(xy[1], xy[2]),
		Point(xy[0], xy[2]),
		Point(xy[0], xy[1]),
		Point(xy[1], xy[1])
	};

	m_hintLeft = new ui::Image();
	m_hintLeft->create(m_hint, Bitmap::load(c_ResourceDockLeft, sizeof(c_ResourceDockLeft), L"png"), false);
	m_hintLeft->setRect(Rect(xy[0], xy[1], xy[0] + 32, xy[1] + 29));
	m_hintLeft->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintRight = new ui::Image();
	m_hintRight->create(m_hint, Bitmap::load(c_ResourceDockRight, sizeof(c_ResourceDockRight), L"png"), false);
	m_hintRight->setRect(Rect(xy[2], xy[1], xy[2] + 32, xy[1] + 29));
	m_hintRight->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintTop = new ui::Image();
	m_hintTop->create(m_hint, Bitmap::load(c_ResourceDockTop, sizeof(c_ResourceDockTop), L"png"), false);
	m_hintTop->setRect(Rect(xy[1], xy[0], xy[1] + 29, xy[0] + 32));
	m_hintTop->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintBottom = new ui::Image();
	m_hintBottom->create(m_hint, Bitmap::load(c_ResourceDockBottom, sizeof(c_ResourceDockBottom), L"png"), false);
	m_hintBottom->setRect(Rect(xy[1], xy[2], xy[1] + 29, xy[2] + 32));
	m_hintBottom->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_pane = new DockPane(this, (DockPane*)0);

	addEventHandler< SizeEvent >(this, &Dock::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &Dock::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Dock::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &Dock::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &Dock::eventDoubleClick);
	addEventHandler< PaintEvent >(this, &Dock::eventPaint);

	return true;
}

void Dock::destroy()
{
	safeDestroy(m_hint);
	ui::Widget::destroy();
}

DockPane* Dock::getPane() const
{
	return m_pane;
}

void Dock::update(const Rect* rc, bool immediate)
{
	Rect innerRect = getInnerRect();

	// Update chains, calculate deferred child widget rectangles.
	std::vector< WidgetRect > widgetRects;
	m_pane->update(innerRect, widgetRects);

	// Update child widgets.
	setChildRects(&widgetRects[0], widgetRects.size());

	// Continue updating widget.
	Widget::update(rc, immediate);
}

void Dock::eventSize(SizeEvent* event)
{
	update();
}

void Dock::eventButtonDown(MouseButtonDownEvent* event)
{
	Point position = event->getPosition();
	Ref< DockPane > pane;

	if ((pane = m_pane->getPaneFromPosition(position)) != nullptr)
	{
		if (pane->hitGripperClose(position))
		{
			if (pane->m_widget)
				pane->m_widget->hide();
			update();
		}

		if (pane->m_widget)
			pane->m_widget->setFocus();

		event->consume();
		return;
	}

	if ((pane = m_pane->getSplitterFromPosition(position)) != nullptr)
	{
		if (pane->hitSplitter(position))
		{
			m_splittingPane = pane;
			setCursor(pane->m_vertical ? CrSizeNS : CrSizeWE);
			setCapture();
		}
		event->consume();
		return;
	}

	event->consume();
}

void Dock::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
	resetCursor();
}

void Dock::eventMouseMove(MouseMoveEvent* event)
{
	Point position = event->getPosition();
	Cursor cursor = CrArrow;

	if (!hasCapture())
	{
		Ref< DockPane > pane = m_pane->getSplitterFromPosition(position);
		if (
			pane &&
			pane->hitSplitter(position)
		)
		{
			cursor = pane->m_vertical ? CrSizeNS : CrSizeWE;
		}
	}
	else
	{
		m_splittingPane->setSplitterPosition(position);
		cursor = m_splittingPane->m_vertical ? CrSizeNS : CrSizeWE;
		update();
	}

	setCursor(cursor);
}

void Dock::eventDoubleClick(MouseDoubleClickEvent* event)
{
	Point position = event->getPosition();

	Ref< DockPane > pane = m_pane->getPaneFromPosition(position);
	if (!pane)
		return;

	if (
		pane->hitGripper(position) &&
		!pane->hitGripperClose(position)
	)
	{
		T_ASSERT(pane->m_detachable);

		Ref< Widget > widget = pane->m_widget;

		pane->detach();

		// Determine size of form.
		Size widgetSize = widget ? widget->getRect().getSize() : Size(0, 0);
		Size preferredSize = widget ? widget->getPreferedSize() : Size(100, 100);
		Size formSize(std::max(widgetSize.cx, preferredSize.cx), std::max(widgetSize.cy, preferredSize.cy));

		// Create floating form.
		Ref< ToolForm > form = new ToolForm();
		form->create(
			this,
			widget ? widget->getText() : L"",
			formSize.cx,
			formSize.cy,
			ToolForm::WsDefault,
			new FloodLayout()
		);

		// Use same icon as ancestor.
		Form* ancestor = dynamic_type_cast< Form* >(getAncestor());
		if (ancestor)
			form->setIcon(ancestor->getIcon());

		form->addEventHandler< MoveEvent >(this, &Dock::eventFormMove);
		form->addEventHandler< NcMouseButtonUpEvent >(this, &Dock::eventFormNcButtonUp);

		// Reparent widget into floating form.
		if (widget)
			widget->setParent(form);

		form->setData(L"WIDGET", widget);
		form->update();
		form->show();

		update();
	}
}

void Dock::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect innerRect = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(innerRect);

	m_pane->draw(canvas);

	event->consume();
}

void Dock::eventFormMove(MoveEvent* event)
{
	Point position = getMousePosition();
	DockPane* pane = m_pane->getPaneFromPosition(position);
	if (pane)
	{
		// Is hint form already visible for this pane?
		if (pane == m_hintDockPane && m_hint->isVisible(false))
			return;
		else
			m_hint->hide();

		Rect rc = pane->getPaneRect();

		// Convert pane rectangle from client to screen space.
		Point tl = clientToScreen(rc.getTopLeft());
		Point br = clientToScreen(rc.getBottomRight());

		// Show hint at center of hovering pane.
		Point center = tl + Size((br.x - tl.x) / 2, (br.y - tl.y) / 2);
		m_hint->setRect(Rect(
			center - Size(c_hintSize / 2, c_hintSize / 2),
			Size(c_hintSize, c_hintSize)
		));
		m_hint->show();

		m_hintDockPane = pane;
		m_hintDockForm = mandatory_non_null_type_cast< ToolForm* >(event->getSender());
	}
	else
	{
		m_hint->hide();
		m_hintDockPane = nullptr;
		m_hintDockForm = nullptr;
	}
}

void Dock::eventFormNcButtonUp(NcMouseButtonUpEvent* event)
{
	Point position = screenToClient(event->getPosition());

	// Ensure hint form isn't visible.
	if (m_hint)
	{
		m_hint->hide();
		m_hintDockPane = nullptr;
		m_hintDockForm = nullptr;
	}

	Ref< DockPane > pane = m_pane->getPaneFromPosition(position);
	if (pane)
	{
		Ref< ToolForm > form = mandatory_non_null_type_cast< ToolForm* >(event->getSender());
		Ref< Widget > widget = form->getData< Widget >(L"WIDGET");
		Size widgetSize = widget->getRect().getSize();

		// Reparent widget back to dock.
		if (widget)
			widget->setParent(this);

		form->destroy();
		form = nullptr;

		// Calculate docking direction.
		int dx = position.x - pane->m_rect.getCenter().x;
		int dy = position.y - pane->m_rect.getCenter().y;

		DockPane::Direction direction;
		int32_t size;

		if (traktor::abs(dx) > traktor::abs(dy))
		{
			direction = dx > 0 ? DockPane::DrEast : DockPane::DrWest;
			size = widgetSize.cx;
		}
		else
		{
			direction = dy > 0 ? DockPane::DrSouth : DockPane::DrNorth;
			size = widgetSize.cy;
		}

		pane->dock(
			widget,
			true,
			direction,
			size
		);

		update();
	}
}

void Dock::eventHintButtonUp(MouseButtonUpEvent* event)
{
	m_hint->hide();

	Ref< Image > hintImage = dynamic_type_cast< Image* >(event->getSender());
	if (!hintImage)
		return;

	T_ASSERT(m_hintDockForm);
	T_ASSERT(m_hintDockPane);

	Ref< Widget > widget = m_hintDockForm->getData< Widget >(L"WIDGET");
	Size widgetSize = widget->getRect().getSize();

	// Reparent widget back to dock.
	if (widget)
		widget->setParent(this);

	m_hintDockForm->destroy();
	m_hintDockForm = nullptr;

	// Calculate docking direction.
	DockPane::Direction direction;
	int32_t size;

	if (hintImage == m_hintLeft)
	{
		direction = DockPane::DrWest;
		size = widgetSize.cx;
	}
	else if (hintImage == m_hintRight)
	{
		direction = DockPane::DrEast;
		size = widgetSize.cx;
	}
	else if (hintImage == m_hintTop)
	{
		direction = DockPane::DrNorth;
		size = widgetSize.cy;
	}
	else/* if (hintImage == m_hintBottom)*/
	{
		direction = DockPane::DrSouth;
		size = widgetSize.cy;
	}

	m_hintDockPane->dock(
		widget,
		true,
		direction,
		size
	);

	update();
}

	}
}
