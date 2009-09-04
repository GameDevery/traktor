#include "Editor/App/LogView.h"
#include "Ui/Bitmap.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/LogList/LogList.h"
#include "I18N/Text.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/LogFilter.h"
#include "Resources/Standard16.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class LogListTarget : public LogTarget
{
public:
	LogListTarget(ui::custom::LogList* logList, ui::custom::LogList::LogLevel logLevel, LogTarget* previousTarget)
	:	m_logList(logList)
	,	m_logLevel(logLevel)
	,	m_previousTarget(previousTarget)
	{
	}

	virtual void log(const std::wstring& str)
	{
		if (m_previousTarget)
			m_previousTarget->log(str);
		m_logList->add(m_logLevel, str);
	}

private:
	Ref< ui::custom::LogList > m_logList;
	ui::custom::LogList::LogLevel m_logLevel;
	LogTarget* m_previousTarget;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.LogView", LogView, ui::Container)

bool LogView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolToggleInfo = gc_new< ui::custom::ToolBarButton >(
		i18n::Text(L"LOG_VIEW_INFO"),
		ui::Command(L"Editor.Log.ToggleLevel"),
		0,
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleWarning = gc_new< ui::custom::ToolBarButton >(
		i18n::Text(L"LOG_VIEW_WARNING"),
		ui::Command(L"Editor.Log.ToggleLevel"),
		1,
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleError = gc_new< ui::custom::ToolBarButton >(
		i18n::Text(L"LOG_VIEW_ERROR"),
		ui::Command(L"Editor.Log.ToggleLevel"),
		2,
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolFilter = gc_new< ui::custom::ToolBar >();
	m_toolFilter->create(this);
	m_toolFilter->addImage(ui::Bitmap::load(c_ResourceLogFilter, sizeof(c_ResourceLogFilter), L"png"), 3);
	m_toolFilter->addImage(ui::Bitmap::load(c_ResourceStandard16, sizeof(c_ResourceStandard16), L"png"), 10);
	m_toolFilter->addItem(m_toolToggleInfo);
	m_toolFilter->addItem(m_toolToggleWarning);
	m_toolFilter->addItem(m_toolToggleError);
	m_toolFilter->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolFilter->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"TOOLBAR_COPY"), ui::Command(L"Editor.Log.Copy"), 7));

	m_toolFilter->addClickEventHandler(ui::createMethodHandler(this, &LogView::eventToolClick));

	m_log = gc_new< ui::custom::LogList >();
	m_log->create(this, ui::WsNone);
	m_log->addButtonDownEventHandler(ui::createMethodHandler(this, &LogView::eventButtonDown));

	m_popup = gc_new< ui::PopupMenu >();
	m_popup->create();
	m_popup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Log.Copy"), i18n::Text(L"LOG_COPY")));
	m_popup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Log.CopyFiltered"), i18n::Text(L"LOG_COPY_FILTERED")));
	m_popup->add(gc_new< ui::MenuItem >(L"-"));
	m_popup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Log.Clear"), i18n::Text(L"LOG_CLEAR_ALL")));

	m_originalTargets[0] = log::info   .getBuffer().getTarget();
	m_originalTargets[1] = log::warning.getBuffer().getTarget();
	m_originalTargets[2] = log::error  .getBuffer().getTarget();

	m_newTargets[0] = new LogListTarget(m_log, ui::custom::LogList::LvInfo, m_originalTargets[0]);
	m_newTargets[1] = new LogListTarget(m_log, ui::custom::LogList::LvWarning, m_originalTargets[1]);
	m_newTargets[2] = new LogListTarget(m_log, ui::custom::LogList::LvError, m_originalTargets[2]);

	log::info   .getBuffer().setTarget(m_newTargets[0]);
	log::warning.getBuffer().setTarget(m_newTargets[1]);
	log::error  .getBuffer().setTarget(m_newTargets[2]);

	return true;
}

void LogView::destroy()
{
	log::error  .getBuffer().setTarget(m_originalTargets[2]);
	log::warning.getBuffer().setTarget(m_originalTargets[1]);
	log::info   .getBuffer().setTarget(m_originalTargets[0]);

	delete m_newTargets[2];
	delete m_newTargets[1];
	delete m_newTargets[0];

	m_popup->destroy();

	ui::Container::destroy();
}

void LogView::eventToolClick(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

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

void LogView::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::MenuItem > selected = m_popup->show(m_log, mouseEvent->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Editor.Log.Copy")
		m_log->copyLog();
	else if (selected->getCommand() == L"Editor.Log.CopyFiltered")
		m_log->copyLog(m_log->getFilter());
	else if (selected->getCommand() == L"Editor.Log.Clear")
		m_log->removeAll();
}

	}
}
