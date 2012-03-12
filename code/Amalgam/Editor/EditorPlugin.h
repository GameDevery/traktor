#ifndef traktor_amalgam_EditorPlugin_H
#define traktor_amalgam_EditorPlugin_H

#include <list>
#include <map>
#include "Amalgam/Editor/Tool/ITargetAction.h"
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
	namespace net
	{

class DiscoveryManager;

	}

	namespace ui
	{
		namespace custom
		{

class ToolBar;
class ToolBarDropDown;

		}
	}

	namespace amalgam
	{

class HostEnumerator;
class Target;
class TargetListControl;
class TargetInstance;
class TargetManager;

class EditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	EditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual bool handleCommand(const ui::Command& command, bool result);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	struct EditTarget
	{
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
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolTargets;
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

	Thread* m_threadTargetManager;
	Thread* m_threadConnectionManager;
	Thread* m_threadTargetActions;

	void collectTargets();

	void eventToolBarClick(ui::Event* event);

	void eventTargetListPlay(ui::Event* event);

	void eventTargetListStop(ui::Event* event);

	void threadTargetManager();

	void threadConnectionManager();

	void threadTargetActions();
};

	}
}

#endif	// traktor_amalgam_EditorPlugin_H
