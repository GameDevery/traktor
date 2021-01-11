#include <iomanip>
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Runtime/Editor/HostEnumerator.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/Ui/ButtonCell.h"
#include "Runtime/Editor/Ui/DropListCell.h"
#include "Runtime/Editor/Ui/ProgressCell.h"
#include "Runtime/Editor/Ui/TargetBuildEvent.h"
#include "Runtime/Editor/Ui/TargetCaptureEvent.h"
#include "Runtime/Editor/Ui/TargetCommandEvent.h"
#include "Runtime/Editor/Ui/TargetInstanceListItem.h"
#include "Runtime/Editor/Ui/TargetMigrateEvent.h"
#include "Runtime/Editor/Ui/TargetPlayEvent.h"
#include "Runtime/Editor/Ui/TargetStopEvent.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor
{
	namespace runtime
	{
		namespace
		{

const int32_t c_performanceLineHeight = 18;
const int32_t c_performanceHeight = 3 * c_performanceLineHeight;
const int32_t c_commandHeight = 22;

const struct
{
	const wchar_t* name;
	Color4ub color;
}
c_markers[] =
{
	{ L"End", Color4ub(255, 255, 120) },
	{ L"Render update", Color4ub(200, 200, 80) },
	{ L"Session", Color4ub(255, 120, 255) },
	{ L"Script GC", Color4ub(200, 80, 200) },
	{ L"Audio", Color4ub(120, 255, 255) },
	{ L"Rumble", Color4ub(80, 200, 200) },
	{ L"Input", Color4ub(255, 120, 120) },
	{ L"State", Color4ub(200, 80, 80) },
	{ L"Physics", Color4ub(120, 255, 120) },
	{ L"Build", Color4ub(80, 200, 80) },
	{ L"Audio update", Color4ub(120, 120, 255) },
	{ L"Flash update", Color4ub(80, 80, 200) },
	{ L"Flash build", Color4ub(80, 200, 80) },
	{ L"Video update", Color4ub(200, 80, 80) },
	{ L"World update", Color4ub(200, 80, 200) },
	{ L"World build", Color4ub(200, 200, 80) },
	{ L"Script", Color4ub(255, 80, 128) }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetInstanceListItem", TargetInstanceListItem, ui::AutoWidgetCell)

TargetInstanceListItem::TargetInstanceListItem(HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_instance(instance)
,	m_lastInstanceState((TargetState)-1)
{
	m_bitmapLogos = new ui::StyleBitmap(L"Runtime.Logos");

	m_progressCell = new ProgressCell();
	m_hostsCell = new DropListCell(hostEnumerator, instance);

	m_playCell = new ButtonCell(new ui::StyleBitmap(L"Runtime.TargetPlay"), ui::Command());
	m_playCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventPlayButtonClick);

	m_buildCell = new ButtonCell(new ui::StyleBitmap(L"Runtime.TargetBuild"), ui::Command());
	m_buildCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventBuildButtonClick);

	m_migrateCell = new ButtonCell(new ui::StyleBitmap(L"Runtime.TargetMigrate"), ui::Command());
	m_migrateCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventMigrateButtonClick);
}

ui::Size TargetInstanceListItem::getSize() const
{
	RefArray< TargetConnection > connections = m_instance->getConnections();
	return ui::Size(128, ui::dpi96(28 + connections.size() * (c_performanceHeight + c_commandHeight)));
}

void TargetInstanceListItem::placeCells(ui::AutoWidget* widget, const ui::Rect& rect)
{
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect;
	controlRect.bottom = rect.top + ui::dpi96(28);

	int32_t logoSize = m_bitmapLogos->getSize().cy;

	if (m_instance->getState() == TsIdle)
	{
		widget->placeCell(
			m_hostsCell,
			ui::Rect(
				controlRect.left + ui::dpi96(130),
				controlRect.getCenter().y - ui::dpi96(10),
				controlRect.right - ui::dpi96(24) * 3 - 12,
				controlRect.getCenter().y + ui::dpi96(10)
			)
		);
	}
	else
	{
		widget->placeCell(
			m_progressCell,
			ui::Rect(
				controlRect.left + logoSize + 10,
				controlRect.getCenter().y - ui::dpi96(8),
				controlRect.right - ui::dpi96(24) * 3 - 8,
				controlRect.getCenter().y + ui::dpi96(8)
			)
		);
	}

	widget->placeCell(
		m_playCell,
		ui::Rect(
			controlRect.right - ui::dpi96(24) * 3 - 4,
			controlRect.top,
			controlRect.right - ui::dpi96(24) * 2 - 4,
			controlRect.bottom
		)
	);
	widget->placeCell(
		m_buildCell,
		ui::Rect(
			controlRect.right - ui::dpi96(24) * 2 - 4,
			controlRect.top,
			controlRect.right - ui::dpi96(24) * 1 - 4,
			controlRect.bottom
		)
	);
	widget->placeCell(
		m_migrateCell,
		ui::Rect(
			controlRect.right - ui::dpi96(24) * 1 - 4,
			controlRect.top,
			controlRect.right - ui::dpi96(24) * 0 - 4,
			controlRect.bottom
		)
	);

	controlRect.top = controlRect.bottom;
	controlRect.bottom = controlRect.top + ui::dpi96(c_performanceHeight);

	m_stopCells.resize(connections.size());
	m_captureCells.resize(connections.size());
	m_editCells.resize(connections.size());

	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		if (!m_stopCells[i])
		{
			m_stopCells[i] = new ButtonCell(new ui::StyleBitmap(L"Runtime.TargetStop"), ui::Command(i));
			m_stopCells[i]->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventStopButtonClick);
		}

		if (!m_captureCells[i])
		{
			m_captureCells[i] = new ButtonCell(new ui::StyleBitmap(L"Runtime.TargetProfile"), ui::Command(i));
			m_captureCells[i]->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventCaptureButtonClick);
		}

		if (!m_editCells[i])
		{
			Ref< ui::Edit > edit = new ui::Edit();
			edit->create(getWidget< ui::AutoWidget >(), L"", ui::WsNone | ui::WsWantAllInput);
			edit->addEventHandler< ui::KeyDownEvent >(this, &TargetInstanceListItem::eventCommandEditKeyDown);
			m_editCells[i] = new ui::ChildWidgetCell(edit);
		}

		widget->placeCell(
			m_stopCells[i],
			ui::Rect(
				controlRect.right - ui::dpi96(24) * 1 - 4,
				controlRect.top,
				controlRect.right - ui::dpi96(24) * 0 - 4,
				(controlRect.top + controlRect.bottom) / 2
			)
		);

		widget->placeCell(
			m_captureCells[i],
			ui::Rect(
				controlRect.right - ui::dpi96(24) * 1 - 4,
				(controlRect.top + controlRect.bottom) / 2,
				controlRect.right - ui::dpi96(24) * 0 - 4,
				controlRect.bottom
			)
		);

		widget->placeCell(
			m_editCells[i],
			ui::Rect(
				controlRect.left,
				controlRect.bottom,
				controlRect.right,
				controlRect.bottom + ui::dpi96(c_commandHeight)
			)
		);

		controlRect = controlRect.offset(0, controlRect.getHeight() + c_commandHeight);
	}

	AutoWidgetCell::placeCells(widget, rect);
}

void TargetInstanceListItem::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();
	const Platform* platform = m_instance->getPlatform();
	const TargetConfiguration* targetConfiguration = m_instance->getTargetConfiguration();
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect; controlRect.bottom = rect.top + ui::dpi96(28);

	canvas.setBackground(ss->getColor(getWidget< ui::AutoWidget >(), L"item-background-color"));
	canvas.fillRect(controlRect);

	ui::Rect performanceRect = rect;
	performanceRect.top = rect.top + ui::dpi96(28);
	performanceRect.bottom = performanceRect.top + ui::dpi96(c_performanceHeight + c_commandHeight);
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		canvas.setBackground(ss->getColor(getWidget< ui::AutoWidget >(), L"item-connection-background-color"));
		canvas.fillRect(performanceRect);
		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setForeground(ss->getColor(getWidget< ui::AutoWidget >(), L"item-seperator-color"));
	canvas.drawLine(rect.left, rect.bottom - 1, rect.right, rect.bottom - 1);

	if (m_instance->getState() != m_lastInstanceState)
	{
		const wchar_t* c_textIds[] =
		{
			L"RUNTIME_STATE_IDLE",
			L"RUNTIME_STATE_BUILDING",
			L"RUNTIME_STATE_DEPLOYING",
			L"RUNTIME_STATE_LAUNCHING",
			L"RUNTIME_STATE_MIGRATING",
			L"RUNTIME_STATE_PENDING"
		};
		T_FATAL_ASSERT(m_instance->getState() < sizeof_array(c_textIds));
		m_progressCell->setText(i18n::Text(c_textIds[m_instance->getState()]));
		m_lastInstanceState = m_instance->getState();
	}

	if (m_instance->getState() != TsIdle)
	{
		int32_t progress = m_instance->getBuildProgress();
		m_progressCell->setProgress(progress);
	}
	else
		m_progressCell->setProgress(-1);

	int32_t logoSize = m_bitmapLogos->getSize().cy;
	canvas.drawBitmap(
		ui::Point(controlRect.left + 2, controlRect.getCenter().y - logoSize / 2),
		ui::Point(platform->getIconIndex() * logoSize, 0),
		ui::Size(logoSize, logoSize),
		m_bitmapLogos,
		ui::BmAlpha
	);

	if (m_instance->getState() == TsIdle)
	{
		ui::Rect textRect = controlRect;
		textRect.left += logoSize + 10;
		textRect.right -= ui::dpi96(24) * 3 - 8;

		canvas.setForeground(ss->getColor(getWidget< ui::AutoWidget >(), L"color"));
		canvas.drawText(textRect, targetConfiguration->getName(), ui::AnLeft, ui::AnCenter);
	}

	ui::Font widgetFont = getWidget< ui::AutoWidget >()->getFont();
	ui::Font performanceFont = widgetFont; performanceFont.setSize(10);
	ui::Font performanceBoldFont = performanceFont; performanceBoldFont.setBold(true);
	ui::Font markerFont = widgetFont; markerFont.setSize(7);

	performanceRect = rect;
	performanceRect.right -= ui::dpi96(34);
	performanceRect.top = rect.top + ui::dpi96(28);
	performanceRect.bottom = performanceRect.top + ui::dpi96(c_performanceHeight);

	canvas.setForeground(ss->getColor(getWidget< ui::AutoWidget >(), L"color"));
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		const TpsRuntime& runtime = connections[i]->getPerformance< TpsRuntime >();

		canvas.setClipRect(performanceRect);

		ui::Rect nameRect = performanceRect;
		nameRect.bottom = nameRect.top + ui::dpi96(c_performanceLineHeight);

		nameRect.left += 6;
		canvas.setFont(performanceBoldFont);
		canvas.drawText(nameRect, connections[i]->getName(), ui::AnLeft, ui::AnCenter);
		canvas.setFont(performanceFont);

		ui::Rect topRect = performanceRect;
		topRect.top = performanceRect.top + ui::dpi96(c_performanceLineHeight);
		topRect.bottom = topRect.top + ui::dpi96(c_performanceLineHeight);

		topRect.left += ui::dpi96(6);
		canvas.drawText(topRect, str(L"%.1f", runtime.fps), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::dpi96(40);
		canvas.drawText(topRect, str(L"Update: %.1f ms", runtime.update * 1000.0f), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::dpi96(120);
		canvas.drawText(topRect, str(L"Build: %.1f ms", runtime.build * 1000.0f), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::dpi96(120);
		canvas.drawText(topRect, str(L"Render: %.1f ms", runtime.render * 1000.0f), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::dpi96(120);
		canvas.drawText(topRect, str(L"GC: %.1f ms", runtime.garbageCollect * 1000.0f), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect = performanceRect;
		middleRect.top = performanceRect.top + ui::dpi96(c_performanceLineHeight) * 2;
		middleRect.bottom = middleRect.top + ui::dpi96(c_performanceLineHeight);

		middleRect.left += ui::dpi96(46);
		canvas.drawText(middleRect, str(L"Physics: %.1f ms", runtime.physics * 1000.0f), ui::AnLeft, ui::AnCenter);

		middleRect.left += ui::dpi96(120);
		canvas.drawText(middleRect, str(L"Input: %.1f ms", runtime.input * 1000.0f), ui::AnLeft, ui::AnCenter);

		middleRect.left += ui::dpi96(120);
		canvas.drawText(middleRect, str(L"Simulate: %d, %.1f%%, %d", (int32_t)runtime.steps, runtime.interval * 100.0f, runtime.collisions), ui::AnLeft, ui::AnCenter);

		performanceRect = performanceRect.offset(0, ui::dpi96(c_performanceHeight + c_commandHeight));
	}

	canvas.resetClipRect();
	canvas.setFont(widgetFont);

	m_playCell->setEnable(m_instance->getState() == TsIdle);
	m_buildCell->setEnable(m_instance->getState() == TsIdle);
	m_migrateCell->setEnable(m_instance->getState() == TsIdle);
}

void TargetInstanceListItem::eventPlayButtonClick(ui::ButtonClickEvent* event)
{
	TargetPlayEvent playEvent(this, m_instance);
	raiseWidgetEvent(&playEvent);
}

void TargetInstanceListItem::eventBuildButtonClick(ui::ButtonClickEvent* event)
{
	TargetBuildEvent buildEvent(this, m_instance);
	raiseWidgetEvent(&buildEvent);
}

void TargetInstanceListItem::eventMigrateButtonClick(ui::ButtonClickEvent* event)
{
	TargetMigrateEvent migrateEvent(this, m_instance);
	raiseWidgetEvent(&migrateEvent);
}

void TargetInstanceListItem::eventStopButtonClick(ui::ButtonClickEvent* event)
{
	TargetStopEvent stopEvent(this, m_instance, event->getCommand().getId());
	raiseWidgetEvent(&stopEvent);
}

void TargetInstanceListItem::eventCaptureButtonClick(ui::ButtonClickEvent* event)
{
	TargetCaptureEvent captureEvent(this, m_instance, event->getCommand().getId());
	raiseWidgetEvent(&captureEvent);
}

void TargetInstanceListItem::eventCommandEditKeyDown(ui::KeyDownEvent* event)
{
	ui::Edit* edit = mandatory_non_null_type_cast< ui::Edit* >(event->getSender());
	int32_t connectionIndex = 0;	// \fixme

	if (event->getVirtualKey() == ui::VkEscape)
	{
		edit->setText(L"");
	}
	else if (event->getVirtualKey() == ui::VkReturn)
	{
		std::wstring command = edit->getText();
		if (!command.empty())
		{
			TargetCommandEvent commandEvent(this, m_instance, connectionIndex, command);
			raiseEvent(&commandEvent);
			edit->setText(L"");
		}
	}
}

	}
}
