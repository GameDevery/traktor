#include "Ui/Custom/ToolBar/ToolBarDropMenu.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarDropMenu", ToolBarDropMenu, ToolBarItem)

ToolBarDropMenu::ToolBarDropMenu(const Command& command, int32_t width, const std::wstring& text, const std::wstring& toolTip)
:	m_command(command)
,	m_width(width)
,	m_text(text)
,	m_toolTip(toolTip)
,	m_hover(false)
,	m_dropPosition(0)
{
}

int32_t ToolBarDropMenu::add(MenuItem* item)
{
	int32_t index = int32_t(m_items.size() - 1);
	m_items.push_back(item);
	return index;
}

bool ToolBarDropMenu::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	RefArray< MenuItem >::iterator i = m_items.begin() + index;
	m_items.erase(i);

	return true;
}

void ToolBarDropMenu::removeAll()
{
	m_items.resize(0);
}

int32_t ToolBarDropMenu::count() const
{
	return int32_t(m_items.size());
}

MenuItem* ToolBarDropMenu::get(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : 0;
}

bool ToolBarDropMenu::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarDropMenu::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	return Size(m_width, imageHeight + 4);
}

void ToolBarDropMenu::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight)
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

	if (m_hover)
	{
		canvas.setBackground(Color4ub(224, 224, 240));
		canvas.fillRect(Rect(at, size));
	}

	canvas.setBackground(getSystemColor(ScWindowBackground));
	canvas.fillRect(rcButton.inflate(1, 1));

	canvas.setBackground(getSystemColor(ScMenuBackground));
	canvas.fillRect(rcButton);

	if (m_hover)
	{
		canvas.setForeground(Color4ub(128, 128, 140));
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
	canvas.drawText(rcText, m_text, AnLeft, AnCenter);

	m_dropPosition = rcButton.left;
	m_menuPosition = Point(at.x, at.y + size.cy);
}

bool ToolBarDropMenu::mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_hover = true;
	return true;
}

void ToolBarDropMenu::mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_hover = false;
}

void ToolBarDropMenu::buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	PopupMenu menu;
	if (menu.create())
	{
		for (size_t i = 0; i < m_items.size(); ++i)
			menu.add(m_items[i]);
		
		Ref< MenuItem > item = menu.show(toolBar, m_menuPosition);
		if (item)
		{
			CommandEvent cmdEvent(toolBar, item, item->getCommand());
			toolBar->raiseEvent(EiClick, &cmdEvent);
		}

		menu.destroy();
		toolBar->update();
	}
}

void ToolBarDropMenu::buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent)
{
}

		}
	}
}
