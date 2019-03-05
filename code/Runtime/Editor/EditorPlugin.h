#pragma once

#include <list>
#include <map>
#include "Runtime/Editor/Deploy/ITargetAction.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPlugin.h"
#include "Ui/Event.h"

namespace traktor
{

class IProcess;

	namespace net
	{

class DiscoveryManager;

	}

	namespace ui
	{

class TimerEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class ToolBarDropMenu;

	}

	namespace runtime
	{

class HostEnumerator;
class Target;
class TargetBrowseEvent;
class TargetBuildEvent;
class TargetCaptureEvent;
class TargetCommandEvent;
class TargetListControl;
class TargetMigrateEvent;
class TargetInstance;
class TargetManager;
class TargetPlayEvent;
class TargetStopEvent;

/*! \brief Amalgam editor plugin.
 * \ingroup Runtime
 */
class EditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	EditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

private:
	struct EditTarget
	{
		Guid guid;
		std::wstring name;
		Ref< const Target > target;
	};

	struct Action
	{
		Ref< ITargetAction::IProgressListener > listener;
		Ref< ITargetAction > action;
	};

	struct ActionChain
	{
		Ref< TargetInstance > targetInstance;
		std::list< Action > actions;
	};

	typedef std::list< ActionChain > action_queue_t;

	editor::IEditor* m_editor;
	Ref< ui::Widget > m_parent;
	Ref< editor::IEditorPageSite > m_site;

	// \name UI
	// \{
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarDropDown > m_toolTargets;
	Ref< ui::ToolBarDropMenu > m_toolTweaks;
	Ref< ui::ToolBarDropDown > m_toolLanguage;
	Ref< TargetListControl > m_targetList;
	// \}

	// \name Tool
	// \{
	std::vector< EditTarget > m_targets;
	RefArray< TargetInstance > m_targetInstances;
	// \}

	// \name Server
	// \{
	Ref< TargetManager > m_targetManager;					//!< Target connection manager.
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< db::ConnectionManager > m_connectionManager;		//!< Remote database connection manager.
	// \}

	// \name Action Worker
	// \{
	Semaphore m_targetActionQueueLock;
	Signal m_targetActionQueueSignal;
	action_queue_t m_targetActionQueue;
	// \}

	Thread* m_threadHostEnumerator;
	Thread* m_threadTargetActions;
	Ref< IProcess > m_pipelineSlaveProcess;

	void updateTargetLists();

	void updateTargetManagers();

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventTargetListBrowse(TargetBrowseEvent* event);

	void eventTargetListBuild(TargetBuildEvent* event);

	void eventTargetListShowProfiler(TargetCaptureEvent* event);

	void eventTargetListMigrate(TargetMigrateEvent* event);

	void eventTargetListPlay(TargetPlayEvent* event);

	void eventTargetListStop(TargetStopEvent* event);

	void eventTargetListCommand(TargetCommandEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void threadHostEnumerator();

	void threadTargetActions();
};

	}
}

