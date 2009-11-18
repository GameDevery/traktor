#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/ToolForm.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Bitmap.h"
#include "Ui/Image.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/MoveEvent.h"

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
	if (!ui::Widget::create(parent))
		return false;

	m_hint = new ToolForm();
	m_hint->create(this, L"Hint", 0, 0, WsNone);
	m_hint->addButtonUpEventHandler(new MethodHandler< Dock >(this, &Dock::eventHintButtonUp));

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
	m_hint->setOutline(p, sizeof(p) / sizeof(Point));

	m_hintLeft = new ui::Image();
	m_hintLeft->create(m_hint, Bitmap::load(c_ResourceDockLeft, sizeof(c_ResourceDockLeft), L"png"), false);
	m_hintLeft->setRect(Rect(xy[0], xy[1], xy[0] + 32, xy[1] + 29));
	m_hintLeft->addButtonUpEventHandler(createMethodHandler(this, &Dock::eventHintButtonUp));

	m_hintRight = new ui::Image();
	m_hintRight->create(m_hint, Bitmap::load(c_ResourceDockRight, sizeof(c_ResourceDockRight), L"png"), false);
	m_hintRight->setRect(Rect(xy[2], xy[1], xy[2] + 32, xy[1] + 29));
	m_hintRight->addButtonUpEventHandler(createMethodHandler(this, &Dock::eventHintButtonUp));

	m_hintTop = new ui::Image();
	m_hintTop->create(m_hint, Bitmap::load(c_ResourceDockTop, sizeof(c_ResourceDockTop), L"png"), false);
	m_hintTop->setRect(Rect(xy[1], xy[0], xy[1] + 29, xy[0] + 32));
	m_hintTop->addButtonUpEventHandler(createMethodHandler(this, &Dock::eventHintButtonUp));

	m_hintBottom = new ui::Image();
	m_hintBottom->create(m_hint, Bitmap::load(c_ResourceDockBottom, sizeof(c_ResourceDockBottom), L"png"), false);
	m_hintBottom->setRect(Rect(xy[1], xy[2], xy[1] + 29, xy[2] + 32));
	m_hintBottom->addButtonUpEventHandler(createMethodHandler(this, &Dock::eventHintButtonUp));

	m_pane = new DockPane(this, (DockPane*)0);

	addSizeEventHandler(createMethodHandler(this, &Dock::eventSize));
	addButtonDownEventHandler(createMethodHandler(this, &Dock::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &Dock::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &Dock::eventMouseMove));
	addDoubleClickEventHandler(createMethodHandler(this, &Dock::eventDoubleClick));
	addPaintEventHandler(createMethodHandler(this, &Dock::eventPaint));

	return true;
}

void Dock::destroy()
{
	if (m_hint)
	{
		m_hint->destroy();
		m_hint = 0;
	}
	ui::Widget::destroy();
}

Ref< DockPane > Dock::getPane()
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
	setChildRects(widgetRects);

	// Continue updating widget.
	Widget::update(rc, immediate);
}

void Dock::eventSize(Event* event)
{
	update();
}

void Dock::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point position = mouseEvent->getPosition();

	Ref< DockPane > pane = m_pane->getFromPosition(position);
	if (pane)
	{
		if (pane->hitGripperClose(position))
		{
			pane->m_widget->hide();
			update();
		}
		else if (pane->hitSplitter(position))
		{
			m_splittingPane = pane;
			setCursor(pane->m_vertical ? CrSizeNS : CrSizeWE);
			setCapture();
		}
		else
		{
			pane->m_widget->setFocus();
		}

		event->consume();
	}
}

void Dock::eventButtonUp(Event* event)
{
	releaseCapture();
	resetCursor();
}

void Dock::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point position = mouseEvent->getPosition();

	if (!hasCapture())
	{
		Ref< DockPane > pane = m_pane->getFromPosition(position);
		if (pane && pane->hitSplitter(position))
		{
			setCursor(pane->m_vertical ? CrSizeNS : CrSizeWE);
		}
	}
	else
	{
		m_splittingPane->setSplitterPosition(position);
		update();
	}
}

void Dock::eventDoubleClick(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point position = mouseEvent->getPosition();

	Ref< DockPane > pane = m_pane->getFromPosition(position);
	if (pane && pane->hitGripper(position) && !pane->hitGripperClose(position))
	{
		T_ASSERT (pane->m_detachable);

		Ref< Widget > widget = pane->m_widget;

		pane->detach();

		// Create floating form.
		Size preferedSize = widget->getPreferedSize();

		Ref< ToolForm > form = new ToolForm();

		form->create(
			this,
			widget->getText(),
			preferedSize.cx,
			preferedSize.cy,
			ToolForm::WsDefault,
			new FloodLayout()
		);

		form->addMoveEventHandler(createMethodHandler(this, &Dock::eventFormMove));
		form->addNcButtonUpEventHandler(createMethodHandler(this, &Dock::eventFormNcButtonUp));

		// Reparent widget into floating form.
		widget->setParent(form);

		form->setData(L"WIDGET", widget);
		form->update();
		form->show();

		update();
	}
}

void Dock::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();
	Rect innerRect = getInnerRect();

	canvas.fillRect(innerRect);

	m_pane->draw(canvas);

	paintEvent->consume();
}

void Dock::eventFormMove(Event* event)
{
	MoveEvent* moveEvent = checked_type_cast< MoveEvent* >(event);
	Point position = getMousePosition();

	Ref< DockPane > pane = m_pane->getFromPosition(position);
	if (pane)
	{
		// Is hint form already visible for this pane?
		if (pane == m_hintDockPane && m_hint->isVisible(false))
		{
			// Bring hint form to foreground.
			m_hint->raise();
			return;
		}
		else
			m_hint->hide();

		Rect rc = pane->getPaneRect();

		m_hint->setRect(Rect(
			rc.left + (rc.getWidth() - c_hintSize) / 2,
			rc.top + (rc.getHeight() - c_hintSize) / 2,
			rc.left + (rc.getWidth() - c_hintSize) / 2 + c_hintSize,
			rc.top + (rc.getHeight() - c_hintSize) / 2 + c_hintSize
		));

		m_hint->show();
		m_hint->raise();

		m_hintDockPane = pane;
		m_hintDockForm = checked_type_cast< ToolForm* >(moveEvent->getSender());
	}
	else
	{
		m_hint->hide();
		m_hintDockPane = 0;
		m_hintDockForm = 0;
	}
}

void Dock::eventFormNcButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point position = screenToClient(mouseEvent->getPosition());

	// Ensure hint form isn't visible.
	if (m_hint)
	{
		m_hint->hide();
		m_hintDockPane = 0;
		m_hintDockForm = 0;
	}

	Ref< DockPane > pane = m_pane->getFromPosition(position);
	if (pane)
	{
		Ref< ToolForm > form = checked_type_cast< ToolForm* >(mouseEvent->getSender());
		Ref< Widget > widget = form->getData< Widget >(L"WIDGET");

		// Reparent widget back to dock.
		widget->setParent(this);

		form->destroy();
		form = 0;

		// Calculate docking direction.
		int dx = position.x - pane->m_rect.getCenter().x;
		int dy = position.y - pane->m_rect.getCenter().y;

		DockPane::Direction direction;
		if (abs(dx) > abs(dy))
			direction = dx > 0 ? DockPane::DrEast : DockPane::DrWest;
		else
			direction = dy > 0 ? DockPane::DrSouth : DockPane::DrNorth;

		pane->dock(
			widget,
			true,
			direction,
			100
		);

		update();
	}
}

void Dock::eventHintButtonUp(Event* event)
{
	MouseEvent* m = static_cast< MouseEvent* >(event);

	m_hint->hide();

	Ref< Image > hintImage = dynamic_type_cast< Image* >(m->getSender());
	if (!hintImage)
		return;

	T_ASSERT (m_hintDockForm);
	T_ASSERT (m_hintDockPane);

	Ref< Widget > widget = m_hintDockForm->getData< Widget >(L"WIDGET");

	// Reparent widget back to dock.
	widget->setParent(this);

	m_hintDockForm->destroy();
	m_hintDockForm = 0;

	// Calculate docking direction.
	DockPane::Direction direction;
	if (hintImage == m_hintLeft)
		direction = DockPane::DrWest;
	else if (hintImage == m_hintRight)
		direction = DockPane::DrEast;
	else if (hintImage == m_hintTop)
		direction = DockPane::DrNorth;
	else if (hintImage == m_hintBottom)
		direction = DockPane::DrSouth;

	m_hintDockPane->dock(
		widget,
		true,
		direction,
		100
	);

	update();
}

	}
}
