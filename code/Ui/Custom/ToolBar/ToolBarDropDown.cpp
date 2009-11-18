#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarDropDown", ToolBarDropDown, ToolBarItem)

ToolBarDropDown::ToolBarDropDown(const Command& command, int32_t width, const std::wstring& toolTip)
:	m_command(command)
,	m_width(width)
,	m_toolTip(toolTip)
,	m_selected(-1)
,	m_hover(false)
,	m_dropPosition(0)
{
}

int32_t ToolBarDropDown::add(const std::wstring& item)
{
	m_items.push_back(item);
	return int32_t(m_items.size() - 1);
}

bool ToolBarDropDown::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	std::vector< std::wstring >::iterator i = m_items.begin() + index;
	m_items.erase(i);

	if (index >= m_selected)
		m_selected = -1;

	return true;
}

void ToolBarDropDown::removeAll()
{
	m_items.resize(0);
	m_selected = -1;
}

int32_t ToolBarDropDown::count() const
{
	return int32_t(m_items.size());
}

std::wstring ToolBarDropDown::get(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : L"";
}

void ToolBarDropDown::select(int32_t index)
{
	m_selected = index;
}

int32_t ToolBarDropDown::getSelected() const
{
	return m_selected;
}

std::wstring ToolBarDropDown::getSelectedItem() const
{
	return get(m_selected);
}

bool ToolBarDropDown::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarDropDown::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	return Size(m_width, imageHeight + 8);
}

void ToolBarDropDown::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight)
{
	Size size = getSize(toolBar, imageWidth, imageHeight);

	Rect rcText(
		at.x + 4,
		at.y + 2,
		at.x + size.cx - 14 - 2,
		at.y + size.cy - 2
	);
	Rect rcButton(
		at.x + size.cx - 14,
		at.y + 1,
		at.x + size.cx - 1,
		at.y + size.cy - 1
	);

	canvas.setBackground(getSystemColor(ScWindowBackground));
	canvas.fillRect(Rect(at, size));

	canvas.setBackground(getSystemColor(ScMenuBackground));
	canvas.fillRect(rcButton);

	if (m_hover)
	{
		canvas.setForeground(Color(128, 128, 140));
		canvas.drawRect(Rect(at, size));
		canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom);
	}

	Point center = rcButton.getCenter();
	Point pnts[] =
	{
		Point(center.x - 2, center.y - 1),
		Point(center.x + 3, center.y - 1),
		Point(center.x,     center.y + 2)
	};

	canvas.setBackground(getSystemColor(ScWindowText));
	canvas.fillPolygon(pnts, 3);

	canvas.setForeground(getSystemColor(ScWindowText));
	canvas.drawText(rcText, getSelectedItem(), AnLeft, AnCenter);

	m_dropPosition = rcButton.left;
	m_menuPosition = Point(at.x, at.y + size.cy);
}

void ToolBarDropDown::mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_hover = true;
}

void ToolBarDropDown::mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_hover = false;
}

void ToolBarDropDown::buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	PopupMenu menu;
	if (menu.create())
	{
		for (size_t i = 0; i < m_items.size(); ++i)
			menu.add(new MenuItem(Command(i), m_items[i]));
		
		Ref< MenuItem > selectedItem = menu.show(toolBar, m_menuPosition);
		if (selectedItem)
		{
			m_selected = selectedItem->getCommand().getId();

			CommandEvent cmdEvent(toolBar, this, m_command);
			toolBar->raiseEvent(EiClick, &cmdEvent);
		}

		menu.destroy();
	}
}

void ToolBarDropDown::buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent)
{
}

		}
	}
}
