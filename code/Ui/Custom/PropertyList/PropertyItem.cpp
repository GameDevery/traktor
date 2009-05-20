#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Bitmap.h"
#include "Ui/Events/CommandEvent.h"
#include "Drawing/Image.h"

// Resources
#include "Resources/Expand.h"
#include "Resources/Collapse.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

Ref< Bitmap > s_imageExpand;
Ref< Bitmap > s_imageCollapse;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PropertyItem", PropertyItem, Object)

PropertyItem::PropertyItem(const std::wstring& text)
:	m_text(text)
,	m_expanded(false)
,	m_selected(false)
{
	if (!s_imageExpand)
		s_imageExpand = Bitmap::load(c_ResourceExpand, sizeof(c_ResourceExpand), L"png");
	if (!s_imageCollapse)
		s_imageCollapse = Bitmap::load(c_ResourceCollapse, sizeof(c_ResourceCollapse), L"png");
}

void PropertyItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& PropertyItem::getText() const
{
	return m_text;
}

void PropertyItem::expand()
{
	if (!m_expanded)
	{
		m_expanded = true;
		showChildrenInPlaceControls(true);
	}
}

void PropertyItem::collapse()
{
	if (m_expanded)
	{
		m_expanded = false;
		showChildrenInPlaceControls(false);
	}
}

bool PropertyItem::isExpanded() const
{
	return m_expanded;
}

bool PropertyItem::isCollapsed() const
{
	return !m_expanded;
}

void PropertyItem::setSelected(bool selected)
{
	m_selected = selected;
}

bool PropertyItem::isSelected() const
{
	return m_selected;
}

int PropertyItem::getDepth() const
{
	int depth = 0;
	for (PropertyItem* parent = m_parent; parent; parent = parent->m_parent)
		++depth;
	return depth;
}

PropertyItem* PropertyItem::getParentItem()
{
	return m_parent;
}

RefList< PropertyItem >& PropertyItem::getChildItems()
{
	return m_childItems;
}

void PropertyItem::setPropertyList(PropertyList* propertyList)
{
	m_propertyList = propertyList;
}

PropertyList* PropertyItem::getPropertyList() const
{
	return m_propertyList;
}

void PropertyItem::notifyUpdate()
{
	if (m_propertyList)
		m_propertyList->update();
}

void PropertyItem::notifyCommand(const Command& command)
{
	if (m_propertyList)
	{
		CommandEvent cmdEvent(m_propertyList, this, command);
		m_propertyList->raiseEvent(EiCommand, &cmdEvent);
	}
}

void PropertyItem::notifyChange()
{
	if (m_propertyList)
	{
		CommandEvent cmdEvent(m_propertyList, this);
		m_propertyList->raiseEvent(EiContentChange, &cmdEvent);
	}
}

void PropertyItem::addChildItem(PropertyItem* childItem)
{
	if (childItem->m_parent == 0)
	{
		m_childItems.push_back(childItem);
		childItem->m_parent = this;
	}
}

void PropertyItem::removeChildItem(PropertyItem* childItem)
{
	if (childItem->getParentItem() == this)
	{
		m_childItems.remove(childItem);
		childItem->m_parent = 0;
	}
}

void PropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
}

void PropertyItem::destroyInPlaceControls()
{
}

void PropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
}

void PropertyItem::showInPlaceControls(bool show)
{
}

void PropertyItem::mouseButtonDown(MouseEvent* event)
{
}

void PropertyItem::mouseButtonUp(MouseEvent* event)
{
}

void PropertyItem::doubleClick(MouseEvent* event)
{
}

void PropertyItem::mouseMove(MouseEvent* event)
{
}

void PropertyItem::paintBackground(Canvas& canvas, const Rect& rc)
{
	if (m_selected)
	{
		canvas.setForeground(Color(240, 240, 250));
		canvas.setBackground(Color(220, 220, 230));
		canvas.fillGradientRect(rc);
	}
	else
	{
		canvas.setBackground(Color(255, 255, 255));
		canvas.fillRect(rc);
	}
}

void PropertyItem::paintText(Canvas& canvas, const Rect& rc)
{
	int depth = getDepth();
	int left = depth * 8;

	if (!m_childItems.empty())
	{
		Bitmap* image = m_expanded ? s_imageCollapse : s_imageExpand;

		int c = (rc.getHeight() - image->getSize().cy) / 2;

		canvas.drawBitmap(
			ui::Point(rc.left + left + 2, rc.top + c),
			ui::Point(0, 0),
			image->getSize(),
			image
		);

		left += image->getSize().cx + 4;
	}

	canvas.drawText(
		rc.inflate(-2 - left / 2, -1).offset(left / 2, 0),
		m_text,
		AnLeft,
		AnCenter
	);
}

void PropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
}

void PropertyItem::showChildrenInPlaceControls(bool show)
{
	for (RefList< PropertyItem >::iterator i = m_childItems.begin(); i != m_childItems.end(); ++i)
	{
		(*i)->showInPlaceControls(show);
		if ((*i)->isExpanded())
			(*i)->showChildrenInPlaceControls(show);
	}
}

		}
	}
}
