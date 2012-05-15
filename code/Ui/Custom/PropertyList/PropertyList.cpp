#include <algorithm>
#include <stack>
#include "Ui/Application.h"
#include "Ui/HierarchicalState.h"
#include "Ui/ScrollBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

enum Modes
{
	MdNone,
	MdMoveSeparator
};

const int c_columnsHeight = 25;
const int c_propertyItemHeight = 20;
const int c_wheelRotationFactor = 2;

std::wstring buildPath(const PropertyItem* item)
{
	std::wstring path = item->getText();
	const PropertyItem* parent = item->getParentItem();
	if (parent)
		path = buildPath(parent) + L"/" + path;
	return path;
}

void recursiveCaptureState(const PropertyItem* item, HierarchicalState* outState)
{
	if (!item)
		return;

	std::wstring path = buildPath(item);
	outState->addState(path, item->isExpanded(), item->isSelected());

	const RefArray< PropertyItem >& children = item->getChildItems();
	for (RefArray< PropertyItem >::const_iterator i = children.begin(); i != children.end(); ++i)
		recursiveCaptureState(*i, outState);
}

void recursiveApplyState(PropertyItem* item, const HierarchicalState* state)
{
	if (!item)
		return;

	RefArray< PropertyItem >& children = item->getChildItems();
	for (RefArray< PropertyItem >::iterator i = children.begin(); i != children.end(); ++i)
		recursiveApplyState(*i, state);

	std::wstring path = buildPath(item);

	if (state->getExpanded(path))
		item->expand();
	else
		item->collapse();

	if (state->getSelected(path))
		item->setSelected(true);
	else
		item->setSelected(false);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PropertyList", PropertyList, Widget)

PropertyList::PropertyList()
:	m_guidResolver(0)
,	m_separator(80)
,	m_mode(MdNone)
,	m_columnHeader(true)
{
	m_columnNames[0] = L"Name";
	m_columnNames[1] = L"Value";
}

bool PropertyList::create(Widget* parent, int style, IPropertyGuidResolver* guidResolver)
{
	if (!Widget::create(parent, style))
		return false;

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addScrollEventHandler(createMethodHandler(this, &PropertyList::eventScroll));
	m_scrollBar->setVisible(false);

	addButtonDownEventHandler(createMethodHandler(this, &PropertyList::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &PropertyList::eventButtonUp));
	addDoubleClickEventHandler(createMethodHandler(this, &PropertyList::eventDoubleClick));
	addMouseMoveEventHandler(createMethodHandler(this, &PropertyList::eventMouseMove));
	addMouseWheelEventHandler(createMethodHandler(this, &PropertyList::eventMouseWheel));
	addSizeEventHandler(createMethodHandler(this, &PropertyList::eventSize));
	addPaintEventHandler(createMethodHandler(this, &PropertyList::eventPaint));

	m_columnHeader = bool((style & WsColumnHeader) == WsColumnHeader);
	m_guidResolver = guidResolver;

	return true;
}

void PropertyList::destroy()
{
	removeAllPropertyItems();
	Widget::destroy();
}

void PropertyList::addPropertyItem(PropertyItem* propertyItem)
{
	m_propertyItems.push_back(propertyItem);
	propertyItem->setPropertyList(this);
	propertyItem->createInPlaceControls(this);
}

void PropertyList::removePropertyItem(PropertyItem* propertyItem)
{
	propertyItem->destroyInPlaceControls();
	propertyItem->setPropertyList(0);
	m_propertyItems.remove(propertyItem);
}

void PropertyList::addPropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem)
{
	parentPropertyItem->addChildItem(propertyItem);
	propertyItem->setPropertyList(this);
	if (parentPropertyItem->isExpanded())
		propertyItem->createInPlaceControls(this);
}

void PropertyList::removePropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem)
{
	propertyItem->destroyInPlaceControls();
	propertyItem->setPropertyList(0);
	parentPropertyItem->removeChildItem(propertyItem);
}

void PropertyList::removeAllPropertyItems()
{
	if (m_propertyItems.empty())
		return;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants);

	// Remove all items prior to destroying in-place controls as UI may issue a redraw for each
	// removed in-place control.
	m_propertyItems.clear();

	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		(*i)->destroyInPlaceControls();
		(*i)->setPropertyList(0);
	}
}

int PropertyList::getPropertyItems(RefArray< PropertyItem >& propertyItems, int flags)
{
	typedef std::pair< RefArray< PropertyItem >::iterator, RefArray< PropertyItem >::iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_propertyItems.begin(), m_propertyItems.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			PropertyItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					propertyItems.push_back(item);
			}
			else
				propertyItems.push_back(item);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != 0 && item->isCollapsed())
					continue;

				RefArray< PropertyItem >& childItems = item->getChildItems();
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

	return int(propertyItems.size());
}

void PropertyList::setSeparator(int separator)
{
	m_separator = separator;
}

int PropertyList::getSeparator() const
{
	return m_separator;
}

void PropertyList::setColumnName(int column, const std::wstring& name)
{
	T_ASSERT (column == 0 || column == 1);
	m_columnNames[column] = name;
}

Ref< PropertyItem > PropertyList::getPropertyItemFromPosition(const Point& position)
{
	int32_t scrollBarOffset = m_scrollBar->getPosition() * c_propertyItemHeight;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	int32_t y = position.y;
	if (m_columnHeader)
	{
		y -= c_columnsHeight;
		if (y < 0)
			return 0;
	}

	int32_t id = (y + scrollBarOffset) / c_propertyItemHeight;
	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		if (id-- <= 0)
			return *i;
	}

	return 0;
}

bool PropertyList::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	if (!m_guidResolver)
		return false;

	return m_guidResolver->resolvePropertyGuid(guid, resolved);
}

Ref< HierarchicalState > PropertyList::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();
	
	state->setScrollPosition(m_scrollBar->getPosition());
	for (RefArray< PropertyItem >::const_iterator i = m_propertyItems.begin(); i != m_propertyItems.end(); ++i)
		recursiveCaptureState(*i, state);

	return state;
}

void PropertyList::applyState(const HierarchicalState* state)
{
	for (RefArray< PropertyItem >::iterator i = m_propertyItems.begin(); i != m_propertyItems.end(); ++i)
		recursiveApplyState(*i, state);

	updateScrollBar();
	m_scrollBar->setPosition(state->getScrollPosition());
}

void PropertyList::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void PropertyList::addCommandEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiCommand, eventHandler);
}

void PropertyList::addChangeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiContentChange, eventHandler);
}

void PropertyList::update(const Rect* rc, bool immediate)
{
	updateScrollBar();
	placeItems();

	Widget::update(rc, immediate);
}

Size PropertyList::getMinimumSize() const
{
	return Size(256, 256);
}

Size PropertyList::getPreferedSize() const
{
	return Size(256, 256);
}

void PropertyList::updateScrollBar()
{
	Rect rc = getInnerRect();

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	int32_t height = rc.getHeight();
	if (m_columnHeader)
		height -= c_columnsHeight;

	int32_t itemCount = int(propertyItems.size());
	int32_t pageCount = height / c_propertyItemHeight;

	int32_t position = m_scrollBar->getPosition();
	if (position > itemCount)
		position = itemCount;

	m_scrollBar->setRange(itemCount);
	m_scrollBar->setPage(pageCount);
	m_scrollBar->setVisible(itemCount > pageCount);
	m_scrollBar->setPosition(position);
}

void PropertyList::placeItems()
{
	int32_t scrollBarOffset = m_scrollBar->getPosition() * c_propertyItemHeight;
	int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferedSize().cx : 0;
	int32_t top = m_columnHeader ? c_columnsHeight : 0;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	std::vector< WidgetRect > childRects;

	// Issue resize of in-place controls on expanded items.
	Rect rcInner = getInnerRect();
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + c_propertyItemHeight - 1
	);
	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		Rect rcValue (rcItem.left + m_separator + 1, rcItem.top, rcItem.right, rcItem.bottom);
		(*i)->resizeInPlaceControls(rcValue, childRects);

		rcItem = rcItem.offset(0, c_propertyItemHeight);
	}

	// Move all children at once.
	setChildRects(childRects);
}

void PropertyList::eventScroll(Event* event)
{
	placeItems();
	update();
	event->consume();
}

void PropertyList::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point p = mouseEvent->getPosition();

	setFocus();

	m_mousePropertyItem = 0;
	if (p.x >= m_separator - 2 && p.x <= m_separator + 2)
	{
		m_mode = MdMoveSeparator;
		setCursor(CrSizeWE);
		setCapture();
		event->consume();
	}
	else
	{
		int32_t scrollBarOffset = m_scrollBar->getPosition() * c_propertyItemHeight;

		int32_t y = mouseEvent->getPosition().y;
		if (m_columnHeader)
		{
			if ((y -= c_columnsHeight) < 0)
				return;
		}

		RefArray< PropertyItem > propertyItems;
		getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

		int32_t id = (y + scrollBarOffset) / c_propertyItemHeight;
		for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
		{
			if (int(std::distance(propertyItems.begin(), i)) == id)
			{
				if (p.x >= m_separator + 2)
				{
					m_mousePropertyItem = *i;
					m_mousePropertyItem->mouseButtonDown(mouseEvent);
				}
				else if (p.x >= (*i)->getDepth() * 8 && p.x <= (*i)->getDepth() * 8 + 12)
				{
					if ((*i)->isExpanded())
						(*i)->collapse();
					else
						(*i)->expand();

					updateScrollBar();
					placeItems();
				}

				if (!(*i)->isSelected())
				{
					(*i)->setSelected(true);
					CommandEvent cmdEvent(this, *i, Command(id));
					raiseEvent(EiSelectionChange, &cmdEvent);
				}
			}
			else if ((*i)->isSelected())
			{
				(*i)->setSelected(false);
				CommandEvent cmdEvent(this, *i, Command(id));
				raiseEvent(EiSelectionChange, &cmdEvent);
			}
		}

		if (m_mousePropertyItem)
		{
			setCapture();
			event->consume();
		}
	}

	update();
}

void PropertyList::eventButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	m_mode = MdNone;

	if (m_mousePropertyItem)
	{
		m_mousePropertyItem->mouseButtonUp(mouseEvent);
		m_mousePropertyItem = 0;
		event->consume();
	}

	if (hasCapture())
		releaseCapture();
}

void PropertyList::eventDoubleClick(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	const Point& position = mouseEvent->getPosition();

	Ref< PropertyItem > propertyItem = getPropertyItemFromPosition(position);
	if (propertyItem)
	{
		if (propertyItem->isExpanded())
			propertyItem->collapse();
		else
			propertyItem->expand();

		updateScrollBar();
		placeItems();

		event->consume();
	}

	update();
}

void PropertyList::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point p = mouseEvent->getPosition();

	if (m_mode == MdMoveSeparator)
	{
		m_separator = p.x;

		setCursor(CrSizeWE);

		placeItems();
		update();

		event->consume();
	}
	else
	{
		if (p.x >= m_separator - 2 && p.x <= m_separator + 2)
		{
			setCursor(CrSizeWE);
			event->consume();
		}
		else if (m_mousePropertyItem)
		{
			m_mousePropertyItem->mouseMove(mouseEvent);
		}
	}
}

void PropertyList::eventMouseWheel(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	int32_t position = m_scrollBar->getPosition();
	position -= mouseEvent->getWheelRotation() * c_wheelRotationFactor;
	m_scrollBar->setPosition(position);

	placeItems();
	update();
}

void PropertyList::eventSize(Event* event)
{
	Rect rc = getInnerRect();

	int32_t scrollWidth = m_scrollBar->getPreferedSize().cx;
	int32_t top = m_columnHeader ? c_columnsHeight : 0;

	m_scrollBar->setRect(Rect(
		rc.right - scrollWidth,
		rc.top + top,
		rc.right,
		rc.bottom
	));

	updateScrollBar();
	placeItems();

	event->consume();
}

void PropertyList::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	
	Canvas& canvas = paintEvent->getCanvas();
	Rect rcInner = getInnerRect();

	int32_t scrollBarOffset = m_scrollBar->getPosition() * c_propertyItemHeight;
	int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferedSize().cx : 0;
	int32_t top = m_columnHeader ? c_columnsHeight : 0;

	// Clear widget background.
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(rcInner);

	// Draw columns.
	if (m_columnHeader)
	{
		canvas.setForeground(Color4ub(255, 255, 255));
		canvas.setBackground(getSystemColor(ScButtonFace));
		canvas.fillGradientRect(Rect(rcInner.left, rcInner.top, rcInner.right, rcInner.top + c_columnsHeight));

		canvas.setForeground(getSystemColor(ScWindowText));
		canvas.drawText(
			Rect(
				rcInner.left + 2, rcInner.top,
				rcInner.left + m_separator - 2, rcInner.top + c_columnsHeight
			),
			m_columnNames[0],
			AnLeft,
			AnCenter
		);

		canvas.setForeground(Color4ub(208, 208, 208));
		canvas.drawLine(
			rcInner.left + m_separator, rcInner.top + 4,
			rcInner.left + m_separator, rcInner.top + c_columnsHeight - 4
		);

		canvas.setForeground(getSystemColor(ScWindowText));
		canvas.drawText(
			Rect(
				rcInner.left + m_separator + 2, rcInner.top,
				rcInner.right, rcInner.top + c_columnsHeight
			),
			m_columnNames[1],
			AnLeft,
			AnCenter
		);
	}

	// Get visible items.
	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	// Draw property items.
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + c_propertyItemHeight - 1
	);
	RefArray< PropertyItem >::iterator i = propertyItems.begin();
	while (rcItem.bottom < top && i != propertyItems.end())
	{
		rcItem = rcItem.offset(0, c_propertyItemHeight);
		++i;
	}
	while (i != propertyItems.end())
	{
		Rect rcText(rcItem.left, rcItem.top, rcItem.left + m_separator, rcItem.bottom);
		Rect rcValue(rcItem.left + m_separator + 1, rcItem.top, rcItem.right, rcItem.bottom);
		
		// Draw item background.
		canvas.setForeground(Color4ub(0, 0, 0));
		(*i)->paintBackground(canvas, rcItem);

		// Draw item text and possible expand image.
		canvas.setClipRect(rcText);
		canvas.setForeground(Color4ub(0, 0, 0));
		(*i)->paintText(canvas, rcText);
		canvas.resetClipRect();

		// Draw item value.
		canvas.setClipRect(rcValue);
		canvas.setForeground(Color4ub(0, 0, 0));
		(*i)->paintValue(canvas, rcValue);
		canvas.resetClipRect();

		// Draw horizontal item separator.
		canvas.setForeground(Color4ub(190, 190, 200));
		canvas.drawLine(Point(rcItem.left, rcItem.bottom), Point(rcItem.right, rcItem.bottom));

		// Draw vertical item separator.
		canvas.drawLine(
			Point(rcItem.left + m_separator, rcItem.top),
			Point(rcItem.left + m_separator, rcItem.bottom)
		);

		rcItem = rcItem.offset(0, c_propertyItemHeight);
		++i;
	}

	paintEvent->consume();
}

		}
	}
}
