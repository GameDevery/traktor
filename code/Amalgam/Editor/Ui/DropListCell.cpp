#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/Ui/DropListCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace amalgam
	{

DropListCell::DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_hostEnumerator(hostEnumerator)
,	m_instance(instance)
{
}

void DropListCell::mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	ui::PopupMenu menu;
	if (menu.create())
	{
		std::wstring platformName = m_instance->getPlatformName();

		int32_t count = m_hostEnumerator->count();
		for (int32_t i = 0; i < count; ++i)
		{
			if (m_hostEnumerator->supportPlatform(i, platformName))
			{
				std::wstring item;
				m_hostEnumerator->getDescription(i, item);
				menu.add(new ui::MenuItem(ui::Command(i), item));
			}
		}

		Ref< ui::MenuItem > selectedItem = menu.show(widget, m_menuPosition);
		if (selectedItem)
			m_instance->setDeployHostId(selectedItem->getCommand().getId());

		menu.destroy();
	}
}

void DropListCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	ui::Size size = rect.getSize();

	ui::Rect rcText(
		rect.left + 4,
		rect.top + 1,
		rect.right - 14 - 2,
		rect.bottom - 1
	);
	ui::Rect rcButton(
		rect.right - 14,
		rect.top,
		rect.right,
		rect.bottom
	);

	canvas.setBackground(ui::getSystemColor(ui::ScWindowBackground));
	canvas.fillRect(rect);

	canvas.setBackground(ui::getSystemColor(ui::ScMenuBackground));
	canvas.fillRect(rcButton);

	canvas.setForeground(Color4ub(128, 128, 140));
	canvas.drawRect(rect);
	canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom - 1);

	ui::Point center = rcButton.getCenter();
	ui::Point pnts[] =
	{
		ui::Point(center.x - 3, center.y - 1),
		ui::Point(center.x + 2, center.y - 1),
		ui::Point(center.x - 1, center.y + 2)
	};

	canvas.setBackground(ui::getSystemColor(ui::ScWindowText));
	canvas.fillPolygon(pnts, 3);

	int32_t deployHostId = m_instance->getDeployHostId();
	if (deployHostId >= 0)
	{
		std::wstring description;
		m_hostEnumerator->getDescription(deployHostId, description);

		canvas.setForeground(ui::getSystemColor(ui::ScWindowText));
		canvas.drawText(rcText, description, ui::AnLeft, ui::AnCenter);
	}

	m_menuPosition = ui::Point(rect.left, rect.bottom);
}

	}
}
