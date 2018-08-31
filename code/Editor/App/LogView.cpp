/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/App/LogView.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class LogListTarget : public ILogTarget
{
public:
	LogListTarget(ui::custom::LogList* logList)
	:	m_logList(logList)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		m_logList->add(threadId, (ui::custom::LogList::LogLevel)(1 << level), str);
	}

private:
	ui::custom::LogList* m_logList;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.LogView", LogView, ui::Container)

LogView::LogView(IEditor* editor)
:	m_editor(editor)
{
}

bool LogView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolToggleInfo = new ui::custom::ToolBarButton(
		i18n::Text(L"LOG_VIEW_INFO"),
		2,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleWarning = new ui::custom::ToolBarButton(
		i18n::Text(L"LOG_VIEW_WARNING"),
		3,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleError = new ui::custom::ToolBarButton(
		i18n::Text(L"LOG_VIEW_ERROR"),
		1,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolFilter = new ui::custom::ToolBar();
	m_toolFilter->create(this);
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Copy"), 1);
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Error"), 1);
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Info"), 1);
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Warning"), 1);
	m_toolFilter->addItem(m_toolToggleInfo);
	m_toolFilter->addItem(m_toolToggleWarning);
	m_toolFilter->addItem(m_toolToggleError);
	m_toolFilter->addItem(new ui::custom::ToolBarSeparator());
	m_toolFilter->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), 0, ui::Command(L"Editor.Log.Copy")));
	m_toolFilter->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &LogView::eventToolClick);

	m_log = new ui::custom::LogList();
	m_log->create(this, ui::WsNone, this);
	m_log->addEventHandler< ui::MouseButtonDownEvent >(this, &LogView::eventButtonDown);

	m_popup = new ui::Menu();
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.Copy"), i18n::Text(L"LOG_COPY")));
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.CopyFiltered"), i18n::Text(L"LOG_COPY_FILTERED")));
	m_popup->add(new ui::MenuItem(L"-"));
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.Clear"), i18n::Text(L"LOG_CLEAR_ALL")));

	m_logTarget = new LogListTarget(m_log);
	return true;
}

void LogView::destroy()
{
	ui::Container::destroy();
}

void LogView::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Editor.Log.ToggleLevel")
	{
		m_log->setFilter(
			(m_toolToggleInfo->isToggled() ? ui::custom::LogList::LvInfo : 0) |
			(m_toolToggleWarning->isToggled() ? ui::custom::LogList::LvWarning : 0) |
			(m_toolToggleError->isToggled() ? ui::custom::LogList::LvError : 0)
		);
	}
	else if (cmd == L"Editor.Log.Copy")
		m_log->copyLog();
}

void LogView::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	Ref< ui::MenuItem > selected = m_popup->show(m_log, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Editor.Log.Copy")
		m_log->copyLog();
	else if (selected->getCommand() == L"Editor.Log.CopyFiltered")
		m_log->copyLog(m_log->getFilter());
	else if (selected->getCommand() == L"Editor.Log.Clear")
		m_log->removeAll();
}

bool LogView::lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(symbolId);
	if (!instance)
		return false;

	outSymbol = instance->getPath();
	return true;
}

	}
}
