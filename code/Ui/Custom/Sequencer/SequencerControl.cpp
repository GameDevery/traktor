#include <limits>
#include <sstream>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/ScrollBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/SequenceItem.h"
#include "Ui/Custom/Sequencer/SequenceGroup.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sequenceHeight = 22;
const int c_endWidth = 200;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SequencerControl", SequencerControl, Widget)

SequencerControl::SequencerControl()
:	m_separator(140)
,	m_timeScale(8)
,	m_length(5000)
,	m_cursor(0)
{
}

bool SequencerControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_scrollBarV = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBarV->addScrollEventHandler(createMethodHandler(this, &SequencerControl::eventScroll));

	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarH->addScrollEventHandler(createMethodHandler(this, &SequencerControl::eventScroll));

	addSizeEventHandler(createMethodHandler(this, &SequencerControl::eventSize));
	addButtonDownEventHandler(createMethodHandler(this, &SequencerControl::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &SequencerControl::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &SequencerControl::eventMouseMove));
	addMouseWheelEventHandler(createMethodHandler(this, &SequencerControl::eventMouseWheel));
	addPaintEventHandler(createMethodHandler(this, &SequencerControl::eventPaint));

	return true;
}

void SequencerControl::setSeparator(int32_t separator)
{
	m_separator = separator;
	updateScrollBars();
}

int32_t SequencerControl::getSeparator() const
{
	return m_separator;
}

void SequencerControl::setTimeScale(int32_t timeScale)
{
	m_timeScale = timeScale;
	updateScrollBars();
}

int32_t SequencerControl::getTimeScale() const
{
	return m_timeScale;
}

void SequencerControl::setLength(int32_t length)
{
	m_length = length;
	updateScrollBars();
}

int32_t SequencerControl::getLength() const
{
	return m_length;
}

void SequencerControl::setCursor(int32_t cursor)
{
	m_cursor = cursor;
}

int32_t SequencerControl::getCursor() const
{
	return m_cursor;
}

void SequencerControl::addSequenceItem(SequenceItem* sequenceItem)
{
	m_sequenceItems.push_back(sequenceItem);
	updateScrollBars();
}

void SequencerControl::removeSequenceItem(SequenceItem* sequenceItem)
{
	if (sequenceItem->getParentItem())
		sequenceItem->getParentItem()->removeChildItem(sequenceItem);
	else
		m_sequenceItems.remove(sequenceItem);

	updateScrollBars();
}

void SequencerControl::removeAllSequenceItems()
{
	m_sequenceItems.clear();
	updateScrollBars();
}

int SequencerControl::getSequenceItems(RefArray< SequenceItem >& sequenceItems, int flags)
{
	typedef std::pair< RefArray< SequenceItem >::iterator, RefArray< SequenceItem >::iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_sequenceItems.begin(), m_sequenceItems.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			SequenceItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					sequenceItems.push_back(item);
			}
			else
				sequenceItems.push_back(item);

			if (flags & GfDescendants)
			{
				if (is_a< SequenceGroup >(item) && (flags & GfExpandedOnly) != 0)
				{
					if (static_cast< SequenceGroup* >(item)->isCollapsed())
						continue;
				}

				RefArray< SequenceItem >& childItems = item->getChildItems();
				if (!childItems.empty())
				{
					stack.push(std::make_pair(
						childItems.begin(),
						childItems.end()
					));
				}
			}
		}
		else
			stack.pop();
	}

	return int(sequenceItems.size());
}

void SequencerControl::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void SequencerControl::addCursorMoveEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiCursorMove, eventHandler);
}

void SequencerControl::addKeyMoveEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiKeyMove, eventHandler);
}

void SequencerControl::addGroupVisibleEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiGroupVisible, eventHandler);
}

void SequencerControl::eventSize(Event* e)
{
	e->consume();

	Rect rc = getInnerRect();

	int scrollWidth = m_scrollBarV->getPreferedSize().cx;
	int scrollHeight = m_scrollBarH->getPreferedSize().cy;

	m_scrollBarV->setRect(Rect(
		rc.right - scrollWidth,
		rc.top,
		rc.right,
		rc.bottom - scrollHeight
	));

	m_scrollBarH->setRect(Rect(
		rc.left + m_separator,
		rc.bottom - scrollHeight,
		rc.right - scrollWidth,
		rc.bottom
	));

	updateScrollBars();
}

void SequencerControl::updateScrollBars()
{
	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems;
	getSequenceItems(sequenceItems, GfDescendants | GfExpandedOnly);

	Size sequences(
		m_separator + m_length / m_timeScale + c_endWidth,
		int(sequenceItems.size() * c_sequenceHeight) + 1
	);

	Rect rc = getInnerRect();

	int scrollWidth = m_scrollBarV->getPreferedSize().cx;
	int scrollHeight = m_scrollBarH->getPreferedSize().cy;

	int overflowV = std::max< int >(0, sequences.cy - rc.getHeight() + scrollHeight);
	m_scrollBarV->setRange(overflowV);
	m_scrollBarV->setEnable(overflowV > 0);
	m_scrollBarV->setPage(c_sequenceHeight);

	int overflowH = std::max< int >(0, sequences.cx - rc.getWidth() + scrollWidth);
	m_scrollBarH->setRange(overflowH);
	m_scrollBarH->setEnable(overflowH > 0);
	m_scrollBarH->setPage(100);
}

void SequencerControl::eventButtonDown(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);
	if (m->getButton() != MouseEvent::BtLeft)
		return;

	bool selectionModified = false;

	// Grab focus, need it to be able to get key events.
	setFocus();

	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems;
	getSequenceItems(sequenceItems, GfDescendants | GfExpandedOnly);

	// If not shift is down we de-select all items.
	if (!(e->getKeyState() & KsShift))
	{
		for (RefArray< SequenceItem >::iterator i = sequenceItems.begin(); i != sequenceItems.end(); ++i)
			selectionModified |= (*i)->setSelected(false);
	}

	Point position = m->getPosition();
	Rect rc = getInnerRect();

	int sequenceId = (position.y + m_scrollBarV->getPosition()) / c_sequenceHeight;
	if (sequenceId >= 0 && sequenceId < int(sequenceItems.size()))
	{
		RefArray< SequenceItem >::iterator i = sequenceItems.begin();
		std::advance(i, sequenceId);

		// Ensure sequence is selected.
		selectionModified |= (*i)->setSelected(true);

		// Issue local mouse down event on sequence item.
		m_mouseTrackItem.rc = Rect(rc.left, 0, rc.right - m_scrollBarV->getPreferedSize().cx, c_sequenceHeight)
			.offset(0, rc.top - m_scrollBarV->getPosition() + c_sequenceHeight * sequenceId);
		m_mouseTrackItem.item = *i;
		m_mouseTrackItem.item->mouseDown(
			this,
			Point(
				m->getPosition().x - m_mouseTrackItem.rc.left,
				m->getPosition().y - m_mouseTrackItem.rc.top
			),
			m_mouseTrackItem.rc,
			m->getButton(),
			m_separator,
			m_scrollBarH->getPosition()
		);
	}

	// Issue selection change event.
	if (selectionModified)
	{
		CommandEvent cmdEvent(this, 0);
		raiseEvent(EiSelectionChange, &cmdEvent);
	}

	if (position.x >= rc.left + m_separator)
	{
		setCapture();

		m_cursor = (position.x - m_separator + m_scrollBarH->getPosition()) * m_timeScale;
		m_cursor = std::max< int >(m_cursor, 0);
		m_cursor = std::min< int >(m_cursor, m_length);

		CommandEvent cmdEvent(this, 0, Command(m_cursor));
		raiseEvent(EiCursorMove, &cmdEvent);
	}

	update();

	e->consume();
}

void SequencerControl::eventButtonUp(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);
	if (!hasCapture())
		return;

	// Issue local mouse up event on tracked sequence item.
	if (m_mouseTrackItem.item)
	{
		m_mouseTrackItem.item->mouseUp(
			this,
			Point(
				m->getPosition().x - m_mouseTrackItem.rc.left,
				m->getPosition().y - m_mouseTrackItem.rc.top
			),
			m_mouseTrackItem.rc,
			m->getButton(),
			m_separator,
			m_scrollBarH->getPosition()
		);
		m_mouseTrackItem.item = 0;
	}

	releaseCapture();

	e->consume();
}

void SequencerControl::eventMouseMove(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);
	if (!hasCapture())
		return;

	// Calculate current cursor display position.
	int scrollOffsetX = m_scrollBarH->getPosition();

	int cursor;
	cursor = (m->getPosition().x - m_separator + scrollOffsetX) * m_timeScale;
	cursor = std::max< int >(cursor, 0);
	cursor = std::min< int >(cursor, m_length);

	if (cursor != m_cursor)
	{
		m_cursor = cursor;
		CommandEvent cmdEvent(this, 0, Command(m_cursor));
		raiseEvent(EiCursorMove, &cmdEvent);
	}

	// Notify track item mouse move.
	if (m_mouseTrackItem.item)
	{
		m_mouseTrackItem.item->mouseMove(
			this,
			Point(
				m->getPosition().x - m_mouseTrackItem.rc.left,
				m->getPosition().y - m_mouseTrackItem.rc.top
			),
			m_mouseTrackItem.rc,
			m->getButton(),
			m_separator,
			m_scrollBarH->getPosition()
		);
	}

	update();

	e->consume();
}

void SequencerControl::eventMouseWheel(Event* e)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(e);
	int32_t wheel = mouseEvent->getWheelRotation();

	m_timeScale = clamp(m_timeScale + wheel, 1, 32);

	updateScrollBars();
	update();
}

void SequencerControl::eventPaint(Event* e)
{
	PaintEvent* p = static_cast< PaintEvent* >(e);
	Canvas& canvas = p->getCanvas();

	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems;
	getSequenceItems(sequenceItems, GfDescendants | GfExpandedOnly);

	// Get component sizes.
	Rect rc = getInnerRect();
	int scrollWidth = m_scrollBarV->getPreferedSize().cx;
	int scrollHeight = m_scrollBarH->getPreferedSize().cy;

	// Get scroll offsets.
	int scrollOffsetX = m_scrollBarH->getPosition();
	int scrollOffsetY = m_scrollBarV->getPosition();

	// Clear background.
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(Rect(rc.left, rc.top, rc.left + m_separator, rc.bottom));
	
	canvas.setBackground(Color4ub(138, 137, 140));
	canvas.setForeground(Color4ub(118, 117, 120));
	canvas.fillRect(Rect(rc.left + m_separator + 64, rc.top, rc.right, rc.bottom));
	canvas.fillGradientRect(Rect(
		rc.left + m_separator,
		rc.top,
		rc.left + m_separator + 64,
		rc.bottom
	), false);

	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(Rect(rc.right - scrollWidth, rc.bottom - scrollHeight, rc.right, rc.bottom));

	// Right sequence edge.
	int end = std::min(m_separator + m_length / m_timeScale - scrollOffsetX, rc.right - scrollWidth);

	// Draw sequences.
	Rect rcSequence(
		rc.left,
		rc.top - scrollOffsetY,
		rc.right - scrollWidth,
		rc.top - scrollOffsetY + c_sequenceHeight
	);
	for (RefArray< SequenceItem >::iterator i = sequenceItems.begin(); i != sequenceItems.end(); ++i)
	{
		canvas.setClipRect(Rect(
			rc.left,
			rc.top,
			end,
			rc.bottom - scrollHeight
		));

		(*i)->paint(this, canvas, rcSequence, m_separator, scrollOffsetX);

		rcSequence = rcSequence.offset(0, c_sequenceHeight);
	}

	canvas.resetClipRect();

	// Draw cursor.
	int x = m_separator + m_cursor / m_timeScale - scrollOffsetX;
	if (x >= m_separator && x < rc.right)
	{
		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawLine(x, rc.top, x, rc.bottom - scrollHeight - 1);
	}

	// Draw time information.
	Rect rcTime(
		rc.left,
		rc.bottom - scrollHeight,
		rc.left + m_separator,
		rc.bottom
	);

	canvas.setBackground(Color4ub(255, 255, 255));
	canvas.fillRect(rcTime);

	std::wstringstream ss;
	ss << m_cursor << L" ms";
	Size ext = canvas.getTextExtent(ss.str());

	canvas.setForeground(Color4ub(0, 0, 0));
	canvas.drawText(
		Point(
			rcTime.left + 4,
			rcTime.top + (rcTime.getHeight() - ext.cy) / 2
		),
		ss.str()
	);

	e->consume();
}

void SequencerControl::eventScroll(Event* e)
{
	update();
	e->consume();
}

		}
	}
}
