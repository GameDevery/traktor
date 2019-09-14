#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Library/Library.h"
#include "Core/Thread/Semaphore.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineBuilder.h"
#include "Ui/Command.h"
#include "Ui/Form.h"

namespace traktor
{

class CommandLine;
class Thread;

	namespace net
	{

class DiscoveryManager;
class StreamServer;

	}

	namespace ui
	{

class ShortcutTable;
class Dock;
class DockPane;
class Menu;
class MenuItem;
class ProgressBar;
class StatusBar;
class Tab;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarMenu;

	}

	namespace db
	{

class ConnectionManager;
class Database;

	}

	namespace editor
	{

class BuildView;
class DatabaseView;
class Document;
class EditorPageSite;
class EditorPluginSite;
class IEditorPage;
class IEditorPageFactory;
class IEditorPluginFactory;
class IEditorTool;
class IObjectEditor;
class IObjectEditorFactory;
class IPipelineDb;
class LogView;
class MRU;
class PipelineAgentsManager;
class PropertiesView;

/*! \brief Main editor form.
 *
 * This is the surrounding form containing editor pages and the
 * database view.
 */
class EditorForm
:	public ui::Form
,	public IEditor
,	public IPipelineBuilder::IListener
{
	T_RTTI_CLASS;

public:
	EditorForm();

	bool create(const CommandLine& cmdLine);

	virtual void destroy() override;

	/*! \name IEditor implementation */
	//@{

	virtual Ref< const PropertyGroup > getSettings() const override final;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() override final;

	virtual void commitGlobalSettings() override final;

	virtual void revertGlobalSettings() override final;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() override final;

	virtual void commitWorkspaceSettings() override final;

	virtual void revertWorkspaceSettings() override final;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< db::Database > getOutputDatabase() const override final;

	virtual void updateDatabaseView() override final;

	virtual bool highlightInstance(const db::Instance* instance) override final;

	virtual const TypeInfo* browseType() override final;

	virtual const TypeInfo* browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable) override final;

	virtual Ref< db::Group > browseGroup() override final;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) override final;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) override final;

	virtual bool openEditor(db::Instance* instance) override final;

	virtual bool openDefaultEditor(db::Instance* instance) override final;

	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) override final;

	virtual bool openBrowser(const net::Url& url) override final;

	virtual Ref< IEditorPage > getActiveEditorPage() override final;

	virtual void setActiveEditorPage(IEditorPage* editorPage) override final;

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) override final;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) override final;

	virtual void buildAssets(bool rebuild) override final;

	virtual void buildCancel() override final;

	virtual void buildWaitUntilFinished() override final;

	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) override final;

	virtual void setStoreObject(const std::wstring& name, Object* object) override final;

	virtual Object* getStoreObject(const std::wstring& name) const override final;

	//@}

	/*! \name IPipelineBuilder::IListener implementation */
	//@{

	virtual void beginBuild(int32_t core, int32_t index, int32_t count, const PipelineDependency* dependency) override final;

	virtual void endBuild(int32_t core, int32_t index, int32_t count, const PipelineDependency* dependency, IPipelineBuilder::BuildResult result) override final;

	//@}

private:
	friend class EditorPageSite;
	friend class EditorPluginSite;

	RefArray< IEditorPageFactory > m_editorPageFactories;
	RefArray< IObjectEditorFactory > m_objectEditorFactories;
	RefArray< IEditorPluginFactory > m_editorPluginFactories;
	RefArray< IEditorTool > m_editorTools;
	RefArray< EditorPluginSite > m_editorPluginSites;
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::StreamServer > m_streamServer;
	Ref< db::ConnectionManager > m_dbConnectionManager;
	Ref< PipelineAgentsManager > m_agentsManager;
	Ref< IPipelineDb > m_pipelineDb;
	std::map< std::wstring, Ref< Object > > m_objectStore;
	Ref< MRU > m_mru;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneWest;
	Ref< ui::DockPane > m_paneEast;
	Ref< ui::DockPane > m_paneSouth;
	Ref< ui::ToolBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemRecent;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::StatusBar > m_statusBar;
	Ref< ui::ProgressBar > m_buildProgress;
	Ref< ui::Tab > m_tab;
	Ref< ui::Menu > m_menuTab;
	Ref< ui::ToolBarMenu > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< PropertiesView > m_propertiesView;
	Ref< ui::Tab > m_tabOutput;
	std::map< std::wstring, Ref< ILogTarget > > m_logTargets;
	Ref< LogView > m_logView;
	Ref< BuildView > m_buildView;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IEditorPage > m_activeEditorPage;
	Ref< Document > m_activeDocument;
	Ref< EditorPageSite > m_activeEditorPageSite;
	Thread* m_threadAssetMonitor;
	Thread* m_threadBuild;
	Semaphore m_lockBuild;
	Path m_settingsPath;
	Path m_workspacePath;
	Ref< PropertyGroup > m_originalSettings;	//!< Traktor.Editor.config + Traktor.Editor.<platform>.config
	Ref< PropertyGroup > m_globalSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config
	Ref< PropertyGroup > m_workspaceSettings;	//!< <Application>.workspace
	Ref< PropertyGroup > m_mergedSettings;		//!< Traktor.Editor.config + Traktor.Editor.<platform>.config + Traktor.Editor.<user>.config + <Application>.workspace
	int32_t m_buildStep;
	std::wstring m_buildStepMessage;

	bool createWorkspace();

	bool openWorkspace();

	bool openWorkspace(const Path& workspacePath);

	void closeWorkspace();

	void setPropertyObject(Object* properties);

	void createAdditionalPanel(ui::Widget* widget, int size, int32_t direction);

	void destroyAdditionalPanel(ui::Widget* widget);

	void showAdditionalPanel(ui::Widget* widget);

	void hideAdditionalPanel(ui::Widget* widget);

	void updateAdditionalPanelMenu();

	void buildAssetsForOpenedEditors();

	void buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild);

	void updateMRU();

	void updateTitle();

	void updateShortcutTable();

	void saveCurrentDocument();

	void saveAllDocuments();

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void findInDatabase();

	void activatePreviousEditor();

	void activateNextEditor();

	void loadModules();

	void loadLanguageDictionaries();

	void checkModified();

	bool currentModified();

	bool anyModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::ShortcutEvent* event);

	void eventMenuClick(ui::ToolBarButtonClickEvent* event);

	void eventToolClicked(ui::ToolBarButtonClickEvent* event);

	void eventTabButtonDown(ui::MouseButtonDownEvent* event);

	void eventTabSelChange(ui::TabSelectionChangeEvent* event);

	void eventTabClose(ui::TabCloseEvent* event);

	void eventClose(ui::CloseEvent* event);

	void eventTimer(ui::TimerEvent* event);

	//@}

	/*! \name Monitor thread methods. */
	//@{

	void threadAssetMonitor();

	//@}

	/*! \name Open workspace thread. */
	//@{

	void threadOpenWorkspace(const Path& workspacePath, int32_t& progress);

	//@}
};

	}
}
