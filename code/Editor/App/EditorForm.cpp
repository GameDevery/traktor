#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogRedirectTarget.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Editor/Asset.h"
#include "Editor/Assets.h"
#include "Editor/IEditorPage.h"
#include "Editor/IEditorPageFactory.h"
#include "Editor/IEditorPlugin.h"
#include "Editor/IEditorPluginFactory.h"
#include "Editor/IEditorTool.h"
#include "Editor/IObjectEditor.h"
#include "Editor/IObjectEditorFactory.h"
#include "Editor/IPipeline.h"
#include "Editor/PipelineDependency.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/App/BrowseGroupDialog.h"
#include "Editor/App/BrowseInstanceDialog.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/BuildView.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/DefaultObjectEditorFactory.h"
#include "Editor/App/Document.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/EditorPageForm.h"
#include "Editor/App/EditorPageSite.h"
#include "Editor/App/EditorPluginSite.h"
#include "Editor/App/LogView.h"
#include "Editor/App/MRU.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/App/NewWorkspaceDialog.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "Editor/App/PropertiesView.h"
#include "Editor/App/QuickOpenDialog.h"
#include "Editor/App/SettingsDialog.h"
#include "Editor/App/Shortcut.h"
#include "Editor/App/ThumbnailGenerator.h"
#include "Editor/App/WebBrowserPage.h"
#include "Editor/App/WorkspaceDialog.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/PipelineAgentsManager.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineBuilderDistributed.h"
#include "Editor/Pipeline/PipelineDbFlat.h"
#include "Editor/Pipeline/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"
#include "I18N/I18N.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Stream/StreamServer.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/MessageBox.h"
#include "Ui/TableLayout.h"
#include "Ui/FloodLayout.h"
#include "Ui/ShortcutTable.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/BackgroundWorkerDialog.h"
#include "Ui/FileDialog.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarMenu.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/ProgressBar.h"
#include "Ui/InputDialog.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"

#if defined(MessageBox)
#	undef MessageBox
#endif

namespace traktor
{
	namespace editor
	{
		namespace
		{

#if defined(_DEBUG)
const wchar_t* c_title = L"Traktor Editor - Debug build";
#else
const wchar_t* c_title = L"Traktor Editor";
#endif

const uint32_t c_offsetFindingPipelines = 10;
const uint32_t c_offsetCollectingDependencies = 20;
const uint32_t c_offsetBuildingAsset = 30;
const uint32_t c_offsetFinished = 100;

class LogRecordTarget : public ILogTarget
{
public:
	void replay(ILogTarget* intoTarget) const
	{
		for (auto log : m_logs)
			intoTarget->log(log.threadId, log.level, log.str.c_str());
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		m_logs.push_back({ threadId, level, str });
	}

private:
	struct Log
	{
		uint32_t threadId;
		int32_t level;
		std::wstring str;
	};

	std::list< Log > m_logs;
};

class OpenWorkspaceStatus : public RefCountImpl< ui::BackgroundWorkerDialog::IWorkerStatus >
{
public:
	OpenWorkspaceStatus(int32_t& step)
	:	m_step(step)
	{
	}

	virtual bool read(int32_t& outStep, std::wstring& outStatus) override final
	{
		outStep = m_step;
		outStatus = i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_PROGRESS_" + toString(m_step), L"...");
		return true;
	}

private:
	int32_t& m_step;
};

class BuildStatus : public RefCountImpl< ui::BackgroundWorkerDialog::IWorkerStatus >
{
public:
	BuildStatus(int32_t& step, std::wstring& message)
	:	m_step(step)
	,	m_message(message)
	{
	}

	virtual bool read(int32_t& outStep, std::wstring& outStatus) override final
	{
		outStep = m_step;
		outStatus = !m_message.empty() ? m_message : L"...";
		return true;
	}

private:
	int32_t& m_step;
	std::wstring& m_message;
};

struct EditorToolPredicate
{
	bool operator () (const IEditorTool* lh, const IEditorTool* rh) const
	{
		return compareIgnoreCase(lh->getDescription(), rh->getDescription()) < 0;
	}
};

bool loadSettings(const Path& pathName, Ref< PropertyGroup >& outOriginalSettings, Ref< PropertyGroup >* outSettings)
{
	Ref< IStream > file;

#if defined(_WIN32)
    std::wstring system = L"win32";
#elif defined(__APPLE__)
    std::wstring system = L"osx";
#elif defined(__LINUX__)
    std::wstring system = L"linux";
#elif defined(__RPI__)
    std::wstring system = L"rpi";
#endif

	std::wstring globalFile = pathName.getPathName();
	std::wstring systemFile = pathName.getPathNameNoExtension() + L"." + system + L"." + pathName.getExtension();

    // Read global properties.
	if ((file = FileSystem::getInstance().open(globalFile, File::FmRead)) != nullptr)
	{
		outOriginalSettings = xml::XmlDeserializer(file, globalFile).readObject< PropertyGroup >();
		file->close();

		if (!outOriginalSettings)
	        log::error << L"Error while parsing properties \"" << globalFile << L"\"" << Endl;
        else
            T_DEBUG(L"Successfully read properties from \"" << globalFile << L"\"");
	}
	else
        log::warning << L"Unable to read global properties \"" << globalFile << L"\"" << Endl;

    // Read system properties.
    if ((file = FileSystem::getInstance().open(systemFile, File::FmRead)) != nullptr)
    {
        Ref< PropertyGroup > systemSettings = xml::XmlDeserializer(file, systemFile).readObject< PropertyGroup >();
        file->close();

        if (systemSettings)
        {
            if (outOriginalSettings)
            {
                outOriginalSettings = outOriginalSettings->merge(systemSettings, PropertyGroup::MmJoin);
                T_ASSERT(outOriginalSettings);
            }
            else
                outOriginalSettings = systemSettings;

            T_DEBUG(L"Successfully read properties from \"" << systemFile << L"\"");
        }
		else
		{
            log::error << L"Error while parsing properties \"" << systemFile << L"\"" << Endl;
			return false;
		}
    }

	if (!outOriginalSettings)
		return false;

	if (outSettings)
	{
		std::wstring userFile = pathName.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + pathName.getExtension();

		*outSettings = DeepClone(outOriginalSettings).create< PropertyGroup >();
		T_FATAL_ASSERT (*outSettings);

		// Read user properties.
		if ((file = FileSystem::getInstance().open(userFile, File::FmRead)) != nullptr)
		{
			Ref< PropertyGroup > userSettings = xml::XmlDeserializer(file, userFile).readObject< PropertyGroup >();
			file->close();

			if (!userSettings)
			{
				log::error << L"Error while parsing properties \"" << userFile << L"\"" << Endl;
				return false;
			}

			*outSettings = (*outSettings)->merge(userSettings, PropertyGroup::MmJoin);
			T_FATAL_ASSERT (*outSettings);
		}
	}

	return true;
}

bool saveGlobalSettings(const Path& pathName, const PropertyGroup* properties)
{
	std::wstring globalFile = pathName.getPathName();

	Ref< IStream > file = FileSystem::getInstance().open(globalFile, File::FmWrite);
	if (!file)
	{
		log::warning << L"Unable to save properties; changes will be lost" << Endl;
		return false;
	}

	bool result = xml::XmlSerializer(file).writeObject(properties);
	file->close();

	return result;
}

bool saveUserSettings(const Path& pathName, const PropertyGroup* properties)
{
	std::wstring userFile = pathName.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + pathName.getExtension();

	Ref< IStream > file = FileSystem::getInstance().open(userFile, File::FmWrite);
	if (!file)
	{
		log::warning << L"Unable to save properties; changes will be lost" << Endl;
		return false;
	}

	bool result = xml::XmlSerializer(file).writeObject(properties);
	file->close();

	return result;
}

Ref< ui::StyleSheet > loadStyleSheet(const Path& pathName)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
		return xml::XmlDeserializer(file, pathName.getPathName()).readObject< ui::StyleSheet >();
	else
		return nullptr;
}

Ref< db::Database > openDatabase(const std::wstring& connectionString, bool create)
{
	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return nullptr;
	}
	return database;
}

Ref< MRU > loadRecent(const std::wstring& recentFile)
{
	Ref< MRU > mru;

	Ref< IStream > file = FileSystem::getInstance().open(recentFile, File::FmRead);
	if (file)
	{
		mru = xml::XmlDeserializer(file, recentFile).readObject< MRU >();
		file->close();
	}

	if (!mru)
		mru = new MRU();

	return mru;
}

void saveRecent(const std::wstring& recentFile, const MRU* mru)
{
	Ref< IStream > file = FileSystem::getInstance().open(recentFile, File::FmWrite);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(mru);
		file->close();
	}
}

bool findShortcutCommandMapping(const PropertyGroup* settings, const std::wstring& command, int& outKeyState, ui::VirtualKey& outVirtualKey)
{
	const PropertyGroup* shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (!shortcutGroup)
		return false;

	std::wstring keyDesc = shortcutGroup->getProperty< std::wstring >(command);
	if (keyDesc.empty())
		return false;

	std::pair< int, ui::VirtualKey > key = parseShortcut(keyDesc);
	if (!key.first && key.second == ui::VkNull)
		return false;

	outKeyState = key.first;
	outVirtualKey = key.second;

	return true;
}

ui::Size getDesktopSizeEstimate()
{
	std::list< ui::Rect > desktopRects;
	ui::Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);

	ui::Size sz(0, 0);
	for (auto rc : desktopRects)
	{
		sz.cx = std::max(sz.cx, rc.right);
		sz.cy = std::max(sz.cy, rc.bottom);
	}

	return sz;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorForm", EditorForm, ui::Form)

EditorForm::EditorForm()
:	m_threadAssetMonitor(0)
,	m_threadBuild(0)
,	m_buildStep(0)
{
}

bool EditorForm::create(const CommandLine& cmdLine)
{
	Ref< ILogTarget > defaultInfoLog = log::info.getGlobalTarget();
	Ref< ILogTarget > defaultWarningLog = log::info.getGlobalTarget();
	Ref< ILogTarget > defaultErrorLog = log::info.getGlobalTarget();

	// Record logging occuring before log view has been propertly initialized.
	Ref< LogRecordTarget > infoLog = new LogRecordTarget();
	Ref< LogRecordTarget > warningLog = new LogRecordTarget();
	Ref< LogRecordTarget > errorLog = new LogRecordTarget();

	log::info.setGlobalTarget(new LogRedirectTarget(defaultInfoLog, infoLog));
	log::warning.setGlobalTarget(new LogRedirectTarget(defaultWarningLog, warningLog));
	log::error.setGlobalTarget(new LogRedirectTarget(defaultErrorLog, errorLog));

	// Default configuration file.
	m_settingsPath = Path(L"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config");

#if defined(__APPLE__)
	// Load configuration from bundle resources.
	bool forceConsole = cmdLine.hasOption(L"console");
	if (!forceConsole)
		m_settingsPath = Path(L"$(BUNDLE_PATH)/Contents/Resources//resources/runtime/configurations/Traktor.Editor.config");
#endif

	// Overridden configuration file.
	if (cmdLine.hasOption('c', L"configuration"))
		m_settingsPath = Path(cmdLine.getOption('c', L"configuration").getString());

	// Resolve absolute path of settings path as loading a workspace change working directory.
	m_settingsPath = FileSystem::getInstance().getAbsolutePath(m_settingsPath);

	// Load editor global settings.
	if (!loadSettings(m_settingsPath, m_originalSettings, &m_globalSettings))
	{
		log::error << L"Unable to load global settings" << Endl;
		return false;
	}

	// Use only global settings as merged settings until workspace has been loaded.
	m_mergedSettings = m_globalSettings;

	// Load editor stylesheet.
	Ref< ui::StyleSheet > styleSheetShared = loadStyleSheet(L"$(TRAKTOR_HOME)/resources/runtime/themes/Shared/StyleSheet.xss");
	if (!styleSheetShared)
	{
		log::error << L"Unable to load shared stylesheet." << Endl;
		return false;
	}

	std::wstring styleSheetName = m_mergedSettings->getProperty< std::wstring >(L"Editor.StyleSheet", L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss");
	Ref< ui::StyleSheet > styleSheet = loadStyleSheet(styleSheetName);
	if (!styleSheet)
	{
		log::error << L"Unable to load stylesheet " << styleSheetName << Endl;
		return false;
	}

	ui::Application::getInstance()->setStyleSheet(
		styleSheetShared->merge(styleSheet)
	);

	// Load dependent modules.
	loadModules();

	// Load dictionaries.
	loadLanguageDictionary();
	loadHelpDictionary();

	// Load recently used files dictionary.
	m_mru = loadRecent(OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Traktor.Editor.mru");

	if (!ui::Form::create(
		c_title,
		ui::dpi96(1280),
		ui::dpi96(900),
		ui::Form::WsDefault,
		new ui::TableLayout(L"100%", L"*,*,100%,*", 0, 0)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::CloseEvent >(this, &EditorForm::eventClose);
	addEventHandler< ui::TimerEvent >(this, &EditorForm::eventTimer);

	// Create shortcut table.
	m_shortcutTable = new ui::ShortcutTable();
	m_shortcutTable->create();
	m_shortcutTable->addEventHandler< ui::ShortcutEvent >(this, &EditorForm::eventShortcut);

	// Create menu bar.
	m_menuBar = new ui::ToolBar();
	m_menuBar->create(this);
	m_menuBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorForm::eventMenuClick);

	m_menuItemRecent = new ui::MenuItem(i18n::Text(L"MENU_FILE_OPEN_RECENT_WORKSPACE"));

	Ref< ui::ToolBarMenu > menuFile = new ui::ToolBarMenu(i18n::Text(L"MENU_FILE"), L"");
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.NewWorkspace"), i18n::Text(L"MENU_FILE_NEW_WORKSPACE")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.OpenWorkspace"), i18n::Text(L"MENU_FILE_OPEN_WORKSPACE")));
	menuFile->add(m_menuItemRecent);
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Save"), i18n::Text(L"MENU_FILE_SAVE")));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.SaveAll"), i18n::Text(L"MENU_FILE_SAVE_ALL")));
	menuFile->add(new ui::MenuItem(L"-"));
	menuFile->add(new ui::MenuItem(ui::Command(L"Editor.Exit"), i18n::Text(L"MENU_FILE_EXIT")));
	m_menuBar->addItem(menuFile);

	Ref< ui::ToolBarMenu > menuEdit = new ui::ToolBarMenu(i18n::Text(L"MENU_EDIT"), L"");
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Undo"), i18n::Text(L"MENU_EDIT_UNDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Redo"), i18n::Text(L"MENU_EDIT_REDO")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Cut"), i18n::Text(L"MENU_EDIT_CUT")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"MENU_EDIT_COPY")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"MENU_EDIT_PASTE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"MENU_EDIT_DELETE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.SelectAll"), i18n::Text(L"MENU_EDIT_SELECT_ALL")));
	menuEdit->add(new ui::MenuItem(L"-"));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Workspace"), i18n::Text(L"MENU_EDIT_WORKSPACE")));
	menuEdit->add(new ui::MenuItem(ui::Command(L"Editor.Settings"), i18n::Text(L"MENU_EDIT_SETTINGS")));
	m_menuBar->addItem(menuEdit);

	Ref< ui::ToolBarMenu > menuView = new ui::ToolBarMenu(i18n::Text(L"MENU_VIEW"), L"");
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewHome"), i18n::Text(L"MENU_VIEW_HOME")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewDatabase"), i18n::Text(L"MENU_VIEW_DATABASE")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewProperties"), i18n::Text(L"MENU_VIEW_PROPERTIES")));
	menuView->add(new ui::MenuItem(ui::Command(L"Editor.ViewLog"), i18n::Text(L"MENU_VIEW_LOG")));
	menuView->add(new ui::MenuItem(L"-"));
	m_menuItemOtherPanels = new ui::MenuItem(i18n::Text(L"MENU_VIEW_OTHER"));
	menuView->add(m_menuItemOtherPanels);
	m_menuBar->addItem(menuView);

	Ref< ui::ToolBarMenu > menuBuild = new ui::ToolBarMenu(i18n::Text(L"MENU_BUILD"), L"");
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Build"), i18n::Text(L"MENU_BUILD_BUILD")));
	menuBuild->add(new ui::MenuItem(ui::Command(L"Editor.Rebuild"), i18n::Text(L"MENU_BUILD_REBUILD")));
	m_menuBar->addItem(menuBuild);

	// Create toolbar.
	m_toolBar = new ui::ToolBar();
	m_toolBar->create(this, ui::WsNone);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Save"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Cut"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Copy"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Paste"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Undo"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Redo"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Build"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.CancelBuild"), 1);

	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_SAVE"), 0, ui::Command(L"Editor.Save")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_CUT"), 1, ui::Command(L"Editor.Cut")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), 2, ui::Command(L"Editor.Copy")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_PASTE"), 3, ui::Command(L"Editor.Paste")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_UNDO"), 4, ui::Command(L"Editor.Undo")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_REDO"), 5, ui::Command(L"Editor.Redo")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_BUILD"), 6, ui::Command(L"Editor.Build")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_CANCEL_BUILD"), 7, ui::Command(L"Editor.CancelBuild")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorForm::eventToolClicked);

	updateTitle();
	updateMRU();

	m_dock = new ui::Dock();
	m_dock->create(this);

	// Define docking panes.
	Ref< ui::DockPane > pane = m_dock->getPane();
	Ref< ui::DockPane > paneCenter, paneLog;

	pane->split(false, ui::dpi96(400), m_paneWest, paneCenter);
	paneCenter->split(false, ui::dpi96(-250), paneCenter, m_paneEast);
	paneCenter->split(true, ui::dpi96(-200), paneCenter, paneLog);
	paneCenter->split(true, ui::dpi96(-200), paneCenter, m_paneSouth);

	// Create panes.
	m_dataBaseView = new DatabaseView(this);
	m_dataBaseView->create(m_dock);
	m_dataBaseView->setText(i18n::Text(L"TITLE_DATABASE"));
	if (!m_mergedSettings->getProperty< bool >(L"Editor.DatabaseVisible"))
		m_dataBaseView->hide();

	m_paneWest->dock(m_dataBaseView, true);

	m_propertiesView = new PropertiesView(this);
	m_propertiesView->create(m_dock);
	m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));
	if (!m_mergedSettings->getProperty< bool >(L"Editor.PropertiesVisible"))
		m_propertiesView->hide();

	m_paneWest->dock(m_propertiesView, true, ui::DockPane::DrSouth, ui::dpi96(300));

	// Create output panel.
	m_tabOutput = new ui::Tab();
	m_tabOutput->create(m_dock, ui::Tab::WsLine | ui::Tab::WsBottom);
	m_tabOutput->setText(i18n::Text(L"TITLE_OUTPUT"));

	Ref< ui::TabPage > tabPageLog = new ui::TabPage();
	tabPageLog->create(m_tabOutput, i18n::Text(L"TITLE_LOG"), new ui::FloodLayout());

	m_logView = new LogView(this);
	m_logView->create(tabPageLog);
	m_logView->setText(i18n::Text(L"TITLE_LOG"));
	if (!m_mergedSettings->getProperty< bool >(L"Editor.LogVisible"))
		m_logView->hide();

	// Replay logs into log view.
	infoLog->replay(m_logView->getLogTarget());
	warningLog->replay(m_logView->getLogTarget());
	errorLog->replay(m_logView->getLogTarget());

	log::info.setGlobalTarget(new LogRedirectTarget(defaultInfoLog, m_logView->getLogTarget()));
	log::warning.setGlobalTarget(new LogRedirectTarget(defaultWarningLog, m_logView->getLogTarget()));
	log::error.setGlobalTarget(new LogRedirectTarget(defaultErrorLog, m_logView->getLogTarget()));

	Ref< ui::TabPage > tabPageBuild = new ui::TabPage();
	tabPageBuild->create(m_tabOutput, i18n::Text(L"TITLE_BUILD"), new ui::FloodLayout());

	m_buildView = new BuildView();
	m_buildView->create(tabPageBuild);

	m_tabOutput->addPage(tabPageLog);
	m_tabOutput->addPage(tabPageBuild);
	m_tabOutput->setActivePage(tabPageLog);

	paneLog->dock(m_tabOutput, true);

	m_tab = new ui::Tab();
	m_tab->create(m_dock, ui::Tab::WsLine | ui::Tab::WsCloseButton);
	m_tab->addImage(new ui::StyleBitmap(L"Editor.Database.Types"), 23);
	m_tab->addEventHandler< ui::MouseButtonDownEvent >(this, &EditorForm::eventTabButtonDown);
	m_tab->addEventHandler< ui::TabSelectionChangeEvent >(this, &EditorForm::eventTabSelChange);
	m_tab->addEventHandler< ui::TabCloseEvent >(this, &EditorForm::eventTabClose);

	paneCenter->dock(m_tab, false);

	// Create tab pop up.
	m_menuTab = new ui::Menu();
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseEditor"), i18n::Text(L"CLOSE")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.CloseAllOtherEditors"), i18n::Text(L"CLOSE_ALL_BUT_THIS")));
	m_menuTab->add(new ui::MenuItem(ui::Command(L"Editor.FindInDatabase"), i18n::Text(L"FIND_IN_DATABASE")));

	// Create status bar.
	m_statusBar = new ui::StatusBar();
	m_statusBar->create(this);
	m_statusBar->setText(i18n::Text(L"STATUS_IDLE"));

	m_buildProgress = new ui::ProgressBar();
	m_buildProgress->create(m_statusBar);
	m_buildProgress->setVisible(false);

	// Create shared discovery manager.
	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(net::MdFindServices | net::MdPublishServices);
	setStoreObject(L"DiscoveryManager", m_discoveryManager);

	// Create editor page factories.
	TypeInfoSet editorPageFactoryTypes;
	type_of< IEditorPageFactory >().findAllOf(editorPageFactoryTypes, false);
	if (!editorPageFactoryTypes.empty())
	{
		for (TypeInfoSet::const_iterator i = editorPageFactoryTypes.begin(); i != editorPageFactoryTypes.end(); ++i)
		{
			Ref< IEditorPageFactory > editorPageFactory = dynamic_type_cast< IEditorPageFactory* >((*i)->createInstance());
			if (editorPageFactory)
				m_editorPageFactories.push_back(editorPageFactory);
		}
	}

	// Create object editor factories.
	TypeInfoSet objectEditorFactoryTypes;
	type_of< IObjectEditorFactory >().findAllOf(objectEditorFactoryTypes, false);
	if (!objectEditorFactoryTypes.empty())
	{
		for (TypeInfoSet::const_iterator i = objectEditorFactoryTypes.begin(); i != objectEditorFactoryTypes.end(); ++i)
		{
			Ref< IObjectEditorFactory > objectEditorFactory = dynamic_type_cast< IObjectEditorFactory* >((*i)->createInstance());
			if (objectEditorFactory)
				m_objectEditorFactories.push_back(objectEditorFactory);
		}
	}

	// Create editor plugin factories.
	TypeInfoSet editorPluginFactoryTypes;
	type_of< IEditorPluginFactory >().findAllOf(editorPluginFactoryTypes, false);
	if (!editorPluginFactoryTypes.empty())
	{
		for (TypeInfoSet::iterator i = editorPluginFactoryTypes.begin(); i != editorPluginFactoryTypes.end(); ++i)
		{
			Ref< IEditorPluginFactory > editorPluginFactory = dynamic_type_cast< IEditorPluginFactory* >((*i)->createInstance());
			if (editorPluginFactory)
				m_editorPluginFactories.push_back(editorPluginFactory);
		}
	}

	// Create editor plugins.
	for (RefArray< IEditorPluginFactory >::iterator i = m_editorPluginFactories.begin(); i != m_editorPluginFactories.end(); ++i)
	{
		Ref< IEditorPlugin > editorPlugin = (*i)->createEditorPlugin(this);
		if (!editorPlugin)
			continue;

		Ref< EditorPluginSite > site = new EditorPluginSite(this, editorPlugin);
		if (site->create(this))
			m_editorPluginSites.push_back(site);
	}

	// Load tools and populate tool menu.
	TypeInfoSet toolTypes;
	type_of< IEditorTool >().findAllOf(toolTypes, false);
	if (!toolTypes.empty())
	{
		m_menuTools = new ui::ToolBarMenu(i18n::Text(L"MENU_TOOLS"), L"");

		for (TypeInfoSet::iterator i = toolTypes.begin(); i != toolTypes.end(); ++i)
		{
			Ref< IEditorTool > tool = dynamic_type_cast< IEditorTool* >((*i)->createInstance());
			if (tool)
				m_editorTools.push_back(tool);
		}

		EditorToolPredicate predicate;
		m_editorTools.sort(predicate);

		for (uint32_t i = 0; i < m_editorTools.size(); ++i)
		{
			std::wstring desc = m_editorTools[i]->getDescription();
			T_ASSERT(!desc.empty());

			m_menuTools->add(new ui::MenuItem(ui::Command(i), desc));

			Ref< ui::IBitmap > toolIcon = m_editorTools[i]->getIcon();
			if (toolIcon)
			{
				int32_t iconIndex = m_toolBar->addImage(toolIcon, 1);
				m_toolBar->addItem(new ui::ToolBarButton(desc, iconIndex, ui::Command(i)));
			}
		}

		if (!m_editorTools.empty())
			m_menuBar->addItem(m_menuTools);
		else
			m_menuTools = nullptr;
	}

	// Collect all shortcut commands from all editors.
	m_shortcutCommands.push_back(ui::Command(L"Editor.Save"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SaveAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllOtherEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CloseAllEditors"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivatePreviousEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ActivateNextEditor"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.SelectAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Copy"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Cut"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Paste"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Undo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Redo"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Delete"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Find"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.FindNext"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Replace"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.ReplaceAll"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Build"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Rebuild"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.CancelBuild"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.QuickOpen"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.ToggleRoot"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.ToggleFavorite"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.Build"));
	m_shortcutCommands.push_back(ui::Command(L"Editor.Database.Rebuild"));

	for (RefArray< IEditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		std::list< ui::Command > editorPageCommands;
		(*i)->getCommands(editorPageCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPageCommands.begin(), editorPageCommands.end());
	}

	for (RefArray< IEditorPluginFactory >::iterator i = m_editorPluginFactories.begin(); i != m_editorPluginFactories.end(); ++i)
	{
		std::list< ui::Command > editorPluginCommands;
		(*i)->getCommands(editorPluginCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), editorPluginCommands.begin(), editorPluginCommands.end());
	}

	for (RefArray< IObjectEditorFactory >::iterator i = m_objectEditorFactories.begin(); i != m_objectEditorFactories.end(); ++i)
	{
		std::list< ui::Command > objectEditorCommands;
		(*i)->getCommands(objectEditorCommands);
		m_shortcutCommands.insert(m_shortcutCommands.end(), objectEditorCommands.begin(), objectEditorCommands.end());
	}

	// Build shortcut accelerator table.
	updateShortcutTable();

	// Create auxiliary tools.
	Path thumbsPath = m_mergedSettings->getProperty< std::wstring >(L"Editor.ThumbsPath");
	setStoreObject(L"ThumbnailGenerator", new ThumbnailGenerator(thumbsPath));

	// Restore last used form settings, if desktop size still match.
	int32_t x = 0, y = 0, width = ui::dpi96(1280), height = ui::dpi96(900);
	bool maximized = false;

	const auto desktopSize = getDesktopSizeEstimate();
	if (
		desktopSize.cx == m_mergedSettings->getProperty< int32_t >(L"Editor.LastDesktopWidth", -1) &&
		desktopSize.cy == m_mergedSettings->getProperty< int32_t >(L"Editor.LastDesktopHeight", -1)
	)
	{
		x = m_mergedSettings->getProperty< int32_t >(L"Editor.PositionX");
		y = m_mergedSettings->getProperty< int32_t >(L"Editor.PositionY");
		width = m_mergedSettings->getProperty< int32_t >(L"Editor.SizeWidth", ui::dpi96(1280));
		height = m_mergedSettings->getProperty< int32_t >(L"Editor.SizeHeight", ui::dpi96(900));
		maximized = m_mergedSettings->getProperty< bool >(L"Editor.Maximized");
	}

	setRect(ui::Rect(x, y, x + width, y + height));
	if (maximized)
		maximize();

	// Open recently used workspace.
	if (m_mergedSettings->getProperty< bool >(L"Editor.AutoOpenRecentlyUsedWorkspace", false))
	{
		Path workspacePath = m_mru->getMostRecentlyUseFile();
		if (!workspacePath.empty())
			openWorkspace(workspacePath);
	}

	// Start thread and timer.
	m_threadAssetMonitor = ThreadManager::getInstance().create(makeFunctor(this, &EditorForm::threadAssetMonitor), L"Asset monitor");
	m_threadAssetMonitor->start();

	startTimer(250);

	// Show form.
	update();
	show();

	return true;
}

void EditorForm::destroy()
{
	// Stop asset monitor thread.
	if (m_threadAssetMonitor)
	{
		while (!m_threadAssetMonitor->stop());
		ThreadManager::getInstance().destroy(m_threadAssetMonitor);
		m_threadAssetMonitor = nullptr;
	}

	closeWorkspace();

	// Destroy all plugins.
	for (auto editorPluginSite : m_editorPluginSites)
		editorPluginSite->destroy();
	m_editorPluginSites.clear();

	// Destroy discovery manager.
	safeDestroy(m_discoveryManager);
	setStoreObject(L"DiscoveryManager", nullptr);

	// Destroy shortcut table.
	safeDestroy(m_shortcutTable);

	// Destroy widgets.
	safeDestroy(m_dock);
	safeDestroy(m_statusBar);
	safeDestroy(m_toolBar);
	safeDestroy(m_menuBar);

	Form::destroy();
}

Ref< const PropertyGroup > EditorForm::getSettings() const
{
	return m_mergedSettings;
}

Ref< PropertyGroup > EditorForm::checkoutGlobalSettings()
{
	return m_globalSettings;
}

void EditorForm::commitGlobalSettings()
{
	if (m_workspaceSettings)
		m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
	else
		m_mergedSettings = m_globalSettings;
}

void EditorForm::revertGlobalSettings()
{
}

Ref< PropertyGroup > EditorForm::checkoutWorkspaceSettings()
{
	return m_workspaceSettings;
}

void EditorForm::commitWorkspaceSettings()
{
	if (m_workspaceSettings)
	{
		m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
		saveGlobalSettings(m_workspacePath, m_workspaceSettings);
	}
	else
		m_mergedSettings = m_globalSettings;
}

void EditorForm::revertWorkspaceSettings()
{
}

Ref< ILogTarget > EditorForm::createLogTarget(const std::wstring& title)
{
	if (m_logTargets[title] == nullptr)
	{
		Ref< ui::TabPage > tabPageLog = new ui::TabPage();
		if (!tabPageLog->create(m_tabOutput, title, new ui::FloodLayout()))
			return nullptr;

		Ref< LogView > logView = new LogView(this);
		logView->create(tabPageLog);
		logView->setText(title);

		Ref< ui::TabPage > activePage = m_tabOutput->getActivePage();
		m_tabOutput->addPage(tabPageLog);

		if (m_globalSettings->getProperty< bool >(L"Editor.ShowNewLogTargets"))
			m_tabOutput->setActivePage(tabPageLog);
		else
			m_tabOutput->setActivePage(activePage);

		m_tabOutput->update();

		m_logTargets[title] = logView->getLogTarget();
	}
	return m_logTargets[title];
}

Ref< db::Database > EditorForm::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< db::Database > EditorForm::getOutputDatabase() const
{
	return m_outputDatabase;
}

void EditorForm::updateDatabaseView()
{
	T_ASSERT(m_dataBaseView);
	m_dataBaseView->updateView();
}

bool EditorForm::highlightInstance(const db::Instance* instance)
{
	T_ASSERT(m_dataBaseView);
	return m_dataBaseView->highlight(instance);
}

const TypeInfo* EditorForm::browseType()
{
	const TypeInfo* type = nullptr;

	BrowseTypeDialog dlgBrowse(m_mergedSettings);
	if (dlgBrowse.create(this, nullptr, false, false))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			type = dlgBrowse.getSelectedType();
		dlgBrowse.destroy();
	}

	return type;
}

const TypeInfo* EditorForm::browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable)
{
	const TypeInfo* type = nullptr;

	BrowseTypeDialog dlgBrowse(m_mergedSettings);
	if (dlgBrowse.create(this, &base, onlyEditable, onlyInstantiable))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			type = dlgBrowse.getSelectedType();
		dlgBrowse.destroy();
	}

	return type;
}

Ref< db::Group > EditorForm::browseGroup()
{
	Ref< db::Group > group;

	BrowseGroupDialog dlgBrowse(this, m_mergedSettings);
	if (dlgBrowse.create(this, m_sourceDatabase))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			group = dlgBrowse.getGroup();
		dlgBrowse.destroy();
	}

	return group;
}

Ref< db::Instance > EditorForm::browseInstance(const TypeInfo& filterType)
{
	TypeInfoSet browseTypes;

	// Lookup which actual types to browse based on filter type; this
	// is used for mapping resources to assets.
	Ref< const PropertyGroup > browseTypeFilter = m_mergedSettings->getProperty< PropertyGroup >(L"Editor.BrowseTypeFilter");
	if (browseTypeFilter)
	{
		Ref< const IPropertyValue > browseTypesSet = browseTypeFilter->getProperty(filterType.getName());
		if (browseTypesSet)
		{
			PropertyStringSet::value_type_t v = PropertyStringSet::get(browseTypesSet);
			for (PropertyStringSet::value_type_t::const_iterator i = v.begin(); i != v.end(); ++i)
			{
				const TypeInfo* browseType = TypeInfo::find(i->c_str());
				if (browseType && browseType->isInstantiable())
					browseTypes.insert(browseType);
			}
		}
	}

	if (browseTypes.empty())
		browseTypes.insert(&filterType);

	editor::TypeBrowseFilter filter(browseTypes);
	return browseInstance(&filter);
}

Ref< db::Instance > EditorForm::browseInstance(const IBrowseFilter* filter)
{
	Ref< db::Instance > instance;

	BrowseInstanceDialog dlgBrowse(this, m_mergedSettings);
	if (dlgBrowse.create(this, m_sourceDatabase, filter))
	{
		if (dlgBrowse.showModal() == ui::DrOk)
			instance = dlgBrowse.getInstance();
		dlgBrowse.destroy();
	}

	if (instance)
	{
		if (m_mergedSettings->getProperty< bool >(L"Editor.BuildAfterBrowseInstance"))
			buildAsset(instance->getGuid(), false);
	}

	return instance;
}

bool EditorForm::openEditor(db::Instance* instance)
{
	T_ASSERT(instance);

	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Activate page if already opened for this instance.
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT(tabPage);

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (document && document->containInstance(instance))
		{
			Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
			setActiveEditorPage(editorPage);
			m_tab->setActivePage(tabPage);
			return true;
		}
	}

	// Checkout instance for exclusive editing.
	if (!instance->checkout())
	{
		log::error << L"Unable to checkout instance \"" << instance->getName() << L"\"" << Endl;
		return false;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		log::error << L"Unable to get object \"" << instance->getName() << L"\"" << Endl;
		instance->revert();
		return false;
	}

	// Find factory supporting instance type.
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< IEditorPageFactory > editorPageFactory;
	Ref< IObjectEditorFactory > objectEditorFactory;

	for (RefArray< IEditorPageFactory >::iterator i = m_editorPageFactories.begin(); i != m_editorPageFactories.end(); ++i)
	{
		const TypeInfoSet typeSet = (*i)->getEditableTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, type_of(object)))
			{
				uint32_t classDifference = type_difference(**j, type_of(object));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					editorPageFactory = *i;
				}
			}
		}
	}

	for (RefArray< IObjectEditorFactory >::iterator i = m_objectEditorFactories.begin(); i != m_objectEditorFactories.end(); ++i)
	{
		const TypeInfoSet typeSet = (*i)->getEditableTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, type_of(object)))
			{
				uint32_t classDifference = type_difference(**j, type_of(object));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					objectEditorFactory = *i;
				}
			}
		}
	}

	// Create new editor page.
	if (editorPageFactory)
	{
		std::set< Guid > dependencies;
		bool needOutputResources;

		// Issue a build if resources need to be up-to-date.
		needOutputResources = editorPageFactory->needOutputResources(type_of(object), dependencies);
		if (!dependencies.empty())
		{
			if (needOutputResources)
				dependencies.insert(instance->getGuid());
			buildAssets(std::vector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		Ref< Document > document = new Document();
		document->editInstance(instance, object);

		Ref< EditorPageSite > site = new EditorPageSite(this, false);

		Ref< IEditorPage > editorPage = editorPageFactory->createEditorPage(this, site, document);
		T_ASSERT(editorPage);

		// Find icon index.
		Ref< PropertyGroup > iconsGroup = m_mergedSettings->getProperty< PropertyGroup >(L"Editor.Icons");
		T_ASSERT(iconsGroup);

		const std::map< std::wstring, Ref< IPropertyValue > >& icons = iconsGroup->getValues();

		int iconIndex = 2;
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = icons.begin(); i != icons.end(); ++i)
		{
			const TypeInfo* iconType = TypeInfo::find(i->first.c_str());
			if (iconType && is_type_of(*iconType, type_of(object)))
			{
				iconIndex = PropertyInteger::get(i->second);
				break;
			}
		}

		// Create tab page container.
		Ref< ui::TabPage > tabPage = new ui::TabPage();
		if (!tabPage->create(m_tab, instance->getName(), iconIndex, new ui::FloodLayout()))
		{
			log::error << L"Failed to create editor; unable to create tab page" << Endl;
			instance->revert();
			return false;
		}

		// Add tab page to tab container.
		m_tab->addPage(tabPage);
		m_tab->update(0, true);

		// Create editor page.
		if (!editorPage->create(tabPage))
		{
			log::error << L"Failed to create editor" << Endl;
			document->close();
			return false;
		}

		// Update tab page in order to ensure layout
		// of child widgets are correct.
		tabPage->update();

		// Save references to editor in tab page's user data.
		tabPage->setData(L"NEEDOUTPUTRESOURCES", new PropertyBoolean(needOutputResources));
		tabPage->setData(L"EDITORPAGEFACTORY", editorPageFactory);
		tabPage->setData(L"EDITORPAGESITE", site);
		tabPage->setData(L"EDITORPAGE", editorPage);
		tabPage->setData(L"DOCUMENT", document);
		tabPage->setData(L"PRIMARY", instance);

		// Activate newly created editor page.
		setActiveEditorPage(editorPage);
	}
	else if (objectEditorFactory)
	{
		std::set< Guid > dependencies;
		bool needOutputResources;

		// Issue a build if resources need to be up-to-date.
		needOutputResources = objectEditorFactory->needOutputResources(type_of(object), dependencies);
		if (needOutputResources || !dependencies.empty())
		{
			if (needOutputResources)
				dependencies.insert(instance->getGuid());
			buildAssets(std::vector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		// Create object editor dialog.
		Ref< ObjectEditorDialog > objectEditorDialog = new ObjectEditorDialog(m_mergedSettings, objectEditorFactory);
		if (!objectEditorDialog->create(this, this, instance, object))
		{
			log::error << L"Failed to create editor" << Endl;
			instance->revert();
			return false;
		}

		objectEditorDialog->show();
	}
	else
	{
		log::error << L"Type \"" << type_name(object) << L"\" not editable" << Endl;
		instance->revert();
	}

	return true;
}

bool EditorForm::openDefaultEditor(db::Instance* instance)
{
	T_ASSERT(instance);

	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Checkout instance for exclusive editing.
	if (!instance->checkout())
	{
		log::error << L"Unable to checkout instance \"" << instance->getName() << L"\"" << Endl;
		return false;
	}

	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		log::error << L"Unable to get object \"" << instance->getName() << L"\"" << Endl;
		instance->revert();
		return false;
	}

	Ref< IObjectEditorFactory > objectEditorFactory = new DefaultObjectEditorFactory();
	T_ASSERT(objectEditorFactory);

	// Open editor dialog.
	Ref< ObjectEditorDialog > objectEditorDialog = new ObjectEditorDialog(m_mergedSettings, objectEditorFactory);
	if (!objectEditorDialog->create(this, this, instance, object))
	{
		log::error << L"Failed to create editor" << Endl;
		instance->revert();
		return false;
	}

	objectEditorDialog->show();
	return true;
}

bool EditorForm::openTool(const std::wstring& toolType, const PropertyGroup* param)
{
	for (RefArray< IEditorTool >::const_iterator i = m_editorTools.begin(); i != m_editorTools.end(); ++i)
	{
		if (type_name(*i) == toolType)
		{
			// Issue a build if resources need to be up-to-date.
			std::set< Guid > dependencies;
			if ((*i)->needOutputResources(dependencies))
			{
				buildAssets(std::vector< Guid >(dependencies.begin(), dependencies.end()), false);
				buildWaitUntilFinished();
			}

			if ((*i)->launch(this, this, param))
				m_dataBaseView->updateView();

			return true;
		}
	}
	return false;
}

bool EditorForm::openBrowser(const net::Url& url)
{
	Ref< ui::TabPage > tabPage = new ui::TabPage();
	tabPage->create(m_tab, url.getString(), 0, new ui::FloodLayout());

	Ref< WebBrowserPage > homePage = new WebBrowserPage(this);
	homePage->create(tabPage, url);

	m_tab->addPage(tabPage);
	m_tab->update(0, true);

	return true;
}

Ref< IEditorPage > EditorForm::getActiveEditorPage()
{
	return m_activeEditorPage;
}

void EditorForm::setActiveEditorPage(IEditorPage* editorPage)
{
	if (editorPage == m_activeEditorPage)
		return;

	int pageCount = m_tab->getPageCount();

	if (m_activeEditorPageSite)
		m_activeEditorPageSite->hide();

	setPropertyObject(0);

	m_activeEditorPage = editorPage;
	m_activeEditorPageSite = nullptr;
	m_activeDocument = nullptr;

	if (m_activeEditorPage)
	{
		for (int i = 0; i < pageCount; ++i)
		{
			Ref< ui::TabPage > page = m_tab->getPage(i);
			if (page->getData< IEditorPage >(L"EDITORPAGE") == m_activeEditorPage)
			{
				m_tab->setActivePage(page);

				m_activeEditorPageSite = page->getData< EditorPageSite >(L"EDITORPAGESITE");
				T_ASSERT(m_activeEditorPageSite);

				m_activeDocument = page->getData< Document >(L"DOCUMENT");
				T_ASSERT(m_activeDocument);

				break;
			}
		}

		if (m_activeEditorPageSite)
			m_activeEditorPageSite->show();
	}

	updateAdditionalPanelMenu();
	updateTitle();
}

bool EditorForm::createWorkspace()
{
	NewWorkspaceDialog newWorkspaceDialog;
	if (!newWorkspaceDialog.create(this))
		return false;

	if (newWorkspaceDialog.showModal() != ui::DrOk || newWorkspaceDialog.getWorkspacePath().empty())
	{
		newWorkspaceDialog.destroy();
		return false;
	}

	newWorkspaceDialog.destroy();

	openWorkspace(newWorkspaceDialog.getWorkspacePath());
	return true;
}

bool EditorForm::openWorkspace()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"EDITOR_BROWSE_WORKSPACE"), L"Workspace files (*.workspace);*.workspace;All files (*.*);*.*"))
		return false;

	Path path;
	if (fileDialog.showModal(path) != ui::DrOk)
	{
		fileDialog.destroy();
		return false;
	}

	bool result = openWorkspace(path);

	fileDialog.destroy();

	return result;
}

bool EditorForm::openWorkspace(const Path& workspacePath)
{
	if (m_workspaceSettings)
		closeWorkspace();

	int32_t progressStep = 0;

	Thread* thread = ThreadManager::getInstance().create(makeFunctor< EditorForm, const Path&, int32_t& >(
		this,
		&EditorForm::threadOpenWorkspace,
		workspacePath,
		progressStep
	), L"Open workspace thread");
	if (!thread)
		return false;

	thread->start();

	// Show a dialog if processing seems to take more than N second(s).
	ui::BackgroundWorkerDialog dialog;
	dialog.create(this, i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_TITLE"), i18n::Text(L"EDITOR_WAIT_OPENING_WORKSPACE_MESSAGE"), false);
	dialog.execute(thread, new OpenWorkspaceStatus(progressStep));
	dialog.destroy();

	ThreadManager::getInstance().destroy(thread);
	thread = nullptr;

	if (!m_sourceDatabase || !m_outputDatabase)
	{
		log::error << L"No databases opened; failed to open workspace." << Endl;
		closeWorkspace();
		return false;
	}

	m_workspacePath = workspacePath;

	// Reload modules, more modules might be added in workspace.
	loadModules();

	// Update UI views.
	updateTitle();
	m_dataBaseView->setDatabase(m_sourceDatabase);

	// Create stream server.
	m_streamServer = new net::StreamServer();
	m_streamServer->create();

	// Create remote database server.
	m_dbConnectionManager = new db::ConnectionManager(m_streamServer);
	m_dbConnectionManager->create();

	// Create pipeline agent manager.
	m_agentsManager = new PipelineAgentsManager(m_discoveryManager, m_streamServer, m_dbConnectionManager);
	m_agentsManager->create(
		m_mergedSettings,
		m_mergedSettings->getProperty< std::wstring >(L"Editor.SourceDatabase"),
		m_mergedSettings->getProperty< std::wstring >(L"Editor.OutputDatabase")
	);

	// Open pipeline database.
	std::wstring pipelineDbConnectionStr = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.Db");

	m_pipelineDb = new PipelineDbFlat();
	m_pipelineDb->open(pipelineDbConnectionStr);

	// Expose servers as stock objects.
	setStoreObject(L"StreamServer", m_streamServer);
	setStoreObject(L"DbConnectionManager", m_dbConnectionManager);
	setStoreObject(L"PipelineAgentsManager", m_agentsManager);

	// Notify plugins about opened workspace.
	for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		(*i)->handleWorkspaceOpened();

	m_mru->usedFile(workspacePath);

	// Create "Home" page.
	std::wstring url = m_mergedSettings->getProperty< std::wstring >(L"Editor.HomeUrl", L"");
	if (!url.empty())
	{
		Ref< ui::TabPage > tabPage = new ui::TabPage();
		tabPage->create(m_tab, i18n::Text(L"EDITOR_HOME"), 0, new ui::FloodLayout());

		Ref< WebBrowserPage > homePage = new WebBrowserPage(this);
		homePage->create(tabPage, url);

		m_tab->addPage(tabPage);
		m_tab->update(0, true);
	}

	saveRecent(OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Traktor.Editor.mru", m_mru);
	updateMRU();

	log::info << L"Workspace opened successfully." << Endl;
	return true;
}

void EditorForm::closeWorkspace()
{
	// Notify plugins about workspace closing.
	for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		(*i)->handleWorkspaceClosed();

	buildCancel();
	closeAllEditors();

	// Ensure all tabs are closed, home tab isn't closed automatically by "close all editors".
	m_tab->removeAllPages();

	// Remove store objects.
	setStoreObject(L"StreamServer", 0);
	setStoreObject(L"DbConnectionManager", 0);
	setStoreObject(L"PipelineAgentsManager", 0);

	// Shutdown agents manager.
	safeDestroy(m_agentsManager);
	safeDestroy(m_dbConnectionManager);
	safeDestroy(m_streamServer);

	// Close pipeline database.
	safeClose(m_pipelineDb);

	// Close databases.
	safeClose(m_outputDatabase);
	safeClose(m_sourceDatabase);

	// Close settings; restore merged as being global.
	m_workspacePath = L"";
	m_workspaceSettings = nullptr;
	m_mergedSettings = m_globalSettings;

	// Update UI views.
	updateTitle();
	m_dataBaseView->setDatabase(0);
}

void EditorForm::setPropertyObject(Object* properties)
{
	m_propertiesView->setPropertyObject(
		dynamic_type_cast< ISerializable* >(properties)
	);

	std::wstring previousTitle = m_propertiesView->getText();
	if (properties)
	{
		StringOutputStream ss;
		ss << i18n::Text(L"TITLE_PROPERTIES").str() << L" - " << type_name(properties);
		m_propertiesView->setText(ss.str());
	}
	else
		m_propertiesView->setText(i18n::Text(L"TITLE_PROPERTIES"));
	if (previousTitle != m_propertiesView->getText())
		m_dock->update();
}

void EditorForm::createAdditionalPanel(ui::Widget* widget, int size, int32_t direction)
{
	T_ASSERT(widget);

	widget->setParent(m_dock);

	if (direction == -1)
	{
		m_paneWest->dock(
			widget,
			true,
			ui::DockPane::DrSouth,
			size
		);
	}
	else if (direction == 1)
	{
		m_paneEast->dock(
			widget,
			true,
			ui::DockPane::DrSouth,
			size
		);
	}
	else
	{
		m_paneSouth->dock(
			widget,
			true,
			ui::DockPane::DrEast,
			size
		);
	}
}

void EditorForm::destroyAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);

	m_paneWest->undock(widget);
	m_paneEast->undock(widget);
	m_paneSouth->undock(widget);
}

void EditorForm::showAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);
	widget->show();
	m_dock->update();
}

void EditorForm::hideAdditionalPanel(ui::Widget* widget)
{
	T_ASSERT(widget);
	widget->hide();
	m_dock->update();
}

void EditorForm::updateAdditionalPanelMenu()
{
	m_menuItemOtherPanels->removeAll();

	if (m_activeEditorPageSite)
	{
		const auto& panelWidgets = m_activeEditorPageSite->getPanelWidgets();
		for (auto i = panelWidgets.begin(); i != panelWidgets.end(); ++i)
		{
			Ref< ui::MenuItem > menuItem = new ui::MenuItem(
				ui::Command(L"Editor.ViewOther", i->first),
				i->first->getText()
			);
			m_menuItemOtherPanels->add(menuItem);
		}
	}
}

void EditorForm::buildAssetsForOpenedEditors()
{
	std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.CachePath");
	std::vector< Guid > assetGuids;

	PipelineFactory pipelineFactory(m_mergedSettings);
	PipelineInstanceCache pipelineInstanceCache(m_sourceDatabase, cachePath);

	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT(tabPage);

		Ref< PropertyBoolean > needOutputResources = tabPage->getData< PropertyBoolean >(L"NEEDOUTPUTRESOURCES");
		if (!needOutputResources || !*needOutputResources)
			continue;

		Ref< IEditorPageFactory > editorPageFactory = tabPage->getData< IEditorPageFactory >(L"EDITORPAGEFACTORY");
		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (!editorPageFactory || !editorPage || !document)
			continue;

		std::set< Guid > dependencies;

		// First add pre-defined resources.
		editorPageFactory->needOutputResources(type_of(document->getInstance(0)), dependencies);

		// Add document instances.
		for (auto instance : document->getInstances())
			dependencies.insert(instance->getGuid());

		assetGuids.insert(assetGuids.end(), dependencies.begin(), dependencies.end());
	}

	if (!assetGuids.empty())
		buildAssets(assetGuids, false);
}

void EditorForm::buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockBuild);

	bool verbose = m_mergedSettings->getProperty< bool >(L"Pipeline.Verbose", false);

	Timer timerBuild;
	timerBuild.start();

	m_buildProgress->setProgress(0);
	m_buildProgress->setProgress(c_offsetFindingPipelines);

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (m_mergedSettings->getProperty< bool >(L"Pipeline.MemCached", false))
	{
		pipelineCache = new editor::MemCachedPipelineCache();
		if (!pipelineCache->create(m_mergedSettings))
		{
			traktor::log::warning << L"Unable to create pipeline memcached cache; cache disabled" << Endl;
			pipelineCache = nullptr;
		}
	}
	if (m_mergedSettings->getProperty< bool >(L"Pipeline.FileCache", false))
	{
		pipelineCache = new editor::FilePipelineCache();
		if (!pipelineCache->create(m_mergedSettings))
		{
			traktor::log::warning << L"Unable to create pipeline file cache; cache disabled" << Endl;
			pipelineCache = nullptr;
		}
	}

	std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.CachePath");

	// Create pipeline factory.
	PipelineFactory pipelineFactory(m_mergedSettings);
	PipelineDependencySet dependencySet;
	PipelineInstanceCache instanceCache(m_sourceDatabase, cachePath);

	// Build dependencies.
	Ref< IPipelineDepends > pipelineDepends;
	if (m_mergedSettings->getProperty< bool >(L"Pipeline.DependsThreads", true))
	{
		pipelineDepends = new PipelineDependsParallel(
			&pipelineFactory,
			m_sourceDatabase,
			&dependencySet,
			m_pipelineDb,
			&instanceCache
		);
	}
	else
	{
		pipelineDepends = new PipelineDependsIncremental(
			&pipelineFactory,
			m_sourceDatabase,
			&dependencySet,
			m_pipelineDb,
			&instanceCache
		);
	}

	log::info << L"Collecting dependencies..." << Endl;
	log::info << IncreaseIndent;

	m_buildProgress->setProgress(c_offsetCollectingDependencies);

	m_pipelineDb->beginTransaction();

	for (std::vector< Guid >::const_iterator i = assetGuids.begin(); i != assetGuids.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);

	log::info << DecreaseIndent;

	pipelineDepends->waitUntilFinished();

	if (verbose)
	{
		double elapsedDependencies = timerBuild.getElapsedTime();
		log::info << L"Collected " << dependencySet.size() << L" dependencies in " << elapsedDependencies << L" second(s)" << Endl;
	}

	m_buildView->beginBuild();

	// Build output.
	Ref< IPipelineBuilder > pipelineBuilder;

	if (
		!m_mergedSettings->getProperty< bool >(L"Pipeline.BuildDistributed", false) ||
		m_agentsManager->getAgentCount() <= 0
	)
		pipelineBuilder = new PipelineBuilder(
			&pipelineFactory,
			m_sourceDatabase,
			m_outputDatabase,
			pipelineCache,
			m_pipelineDb,
			&instanceCache,
			this,
			m_mergedSettings->getProperty< bool >(L"Pipeline.BuildThreads", true),
			verbose
		);
	else
		pipelineBuilder = new PipelineBuilderDistributed(
			m_agentsManager,
			&pipelineFactory,
			m_pipelineDb,
			this
		);

	if (rebuild)
		log::info << L"Rebuilding " << dependencySet.size() << L" asset(s)..." << Endl;
	else
		log::info << L"Building " << dependencySet.size() << L" asset(s)..." << Endl;

	log::info << IncreaseIndent;

	pipelineBuilder->build(&dependencySet, rebuild);

	m_pipelineDb->endTransaction();

	m_buildView->endBuild();

	if (pipelineCache)
		pipelineCache->destroy();

	double elapsedTotal = timerBuild.getElapsedTime();

	uint32_t seconds = uint32_t(elapsedTotal + 0.5);
	uint32_t minutes = seconds / 60; seconds %= 60;
	uint32_t hours = minutes / 60; minutes %= 60;

	log::info << DecreaseIndent;
	log::info << L"Finished (" << hours << L":" << minutes << L":" << seconds << L")" << Endl;
}

void EditorForm::buildCancel()
{
	if (!m_threadBuild)
		return;

	if (!m_threadBuild->stop(0))
	{
		// Keep processing UI events until build has finished.
		setEnable(false);
		while (!m_threadBuild->wait(10))
		{
			ui::Application::getInstance()->process();
		}
		setEnable(true);
	}
	ThreadManager::getInstance().destroy(m_threadBuild);
	m_threadBuild = nullptr;
}

void EditorForm::buildWaitUntilFinished()
{
	if (!m_threadBuild)
		return;

	// Show a dialog if processing seems to take more than N second(s).
	ui::BackgroundWorkerDialog dialog;
	dialog.create(this, i18n::Text(L"EDITOR_WAIT_BUILDING_TITLE"), i18n::Text(L"EDITOR_WAIT_BUILDING_MESSAGE"), false);
	dialog.execute(m_threadBuild, new BuildStatus(m_buildStep, m_buildStepMessage));
	dialog.destroy();

	// As build thread is no longer in use we can safely release it's resources.
	ThreadManager::getInstance().destroy(m_threadBuild);
	m_threadBuild = nullptr;
}

void EditorForm::buildAssets(const std::vector< Guid >& assetGuids, bool rebuild)
{
	if (!m_workspaceSettings)
		return;

	// Stop current build if any.
	buildCancel();

	// Create build thread.
	m_threadBuild = ThreadManager::getInstance().create(
		makeFunctor<
			EditorForm,
			std::vector< Guid >,
			bool
		>(
			this,
			&EditorForm::buildAssetsThread,
			assetGuids,
			rebuild
		),
		L"Pipeline thread"
	);
	if (m_threadBuild)
	{
		m_threadBuild->start(Thread::Above);
		m_statusBar->setText(i18n::Text(L"STATUS_BUILDING"));
	}
}

void EditorForm::buildAsset(const Guid& assetGuid, bool rebuild)
{
	std::vector< Guid > assetGuids;
	assetGuids.push_back(assetGuid);
	buildAssets(assetGuids, rebuild);
}

void EditorForm::buildAssets(bool rebuild)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	if (!m_workspaceSettings)
		return;

	// Automatically save all opened instances.
	if (m_mergedSettings->getProperty< bool >(L"Editor.AutoSave", false))
		saveAllDocuments();

	// Collect root assets.
	log::info << L"Collecting assets..." << Endl;
	log::info << IncreaseIndent;

	std::vector< Guid > assetGuids;
	std::vector< std::wstring > rootInstances = m_workspaceSettings->getProperty< std::vector< std::wstring > >(L"Editor.RootInstances");
	for (std::vector< std::wstring >::const_iterator i = rootInstances.begin(); i != rootInstances.end(); ++i)
		assetGuids.push_back(Guid(*i));

	log::info << DecreaseIndent;

	// Launch build.
	buildAssets(assetGuids, rebuild);
}

Ref< IPipelineDependencySet > EditorForm::buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth)
{
	T_ASSERT(m_sourceDatabase);

	Ref< IPipelineDependencySet > dependencySet = new PipelineDependencySet();

	std::wstring cachePath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.CachePath");

	PipelineFactory pipelineFactory(m_mergedSettings);
	PipelineInstanceCache instanceCache(m_sourceDatabase, cachePath);

	PipelineDependsIncremental pipelineDepends(
		&pipelineFactory,
		m_sourceDatabase,
		dependencySet,
		m_pipelineDb,
		&instanceCache,
		recursionDepth
	);
	pipelineDepends.addDependency(asset);
	pipelineDepends.waitUntilFinished();

	return dependencySet;
}

void EditorForm::setStoreObject(const std::wstring& name, Object* object)
{
	m_objectStore[name] = object;
}

Object* EditorForm::getStoreObject(const std::wstring& name) const
{
	std::map< std::wstring, Ref< Object > >::const_iterator i = m_objectStore.find(name);
	return i != m_objectStore.end() ? i->second : 0;
}

void EditorForm::beginBuild(int32_t core, int32_t index, int32_t count, const PipelineDependency* dependency)
{
	showProgress(c_offsetBuildingAsset + (index * (c_offsetFinished - c_offsetBuildingAsset)) / count, 100);
	m_buildView->beginBuild(core, dependency->outputPath);
	m_buildProgress->setProgress(c_offsetBuildingAsset + (index * (c_offsetFinished - c_offsetBuildingAsset)) / count);
	m_buildStep = (index * 1000) / count;
	m_buildStepMessage = dependency->outputPath;
}

void EditorForm::endBuild(int32_t core, int32_t index, int32_t count, const PipelineDependency* dependency, IPipelineBuilder::BuildResult result)
{
	hideProgress();
	m_buildView->endBuild(core, result);
}

void EditorForm::updateMRU()
{
	m_menuItemRecent->removeAll();

	std::vector< Path > recentFiles;
	m_mru->getUsedFiles(recentFiles);

	for (std::vector< Path >::const_iterator i = recentFiles.begin(); i != recentFiles.end(); ++i)
	{
		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Editor.OpenRecentWorkspace", new Path(*i)), i->getPathName());
		m_menuItemRecent->add(menuItem);
	}
}

void EditorForm::updateTitle()
{
	std::wstringstream ss;

	std::wstring targetTitle = m_mergedSettings->getProperty< std::wstring >(L"Editor.TargetTitle");
	if (!targetTitle.empty())
		ss << targetTitle << L" - ";

	ss << c_title;

	if (m_activeDocument && m_activeDocument->getInstanceCount() > 0)
		ss << L" - " << m_activeDocument->getInstance(0)->getPath();

	setText(ss.str());
}

void EditorForm::updateShortcutTable()
{
	m_shortcutTable->removeAllCommands();

	for (std::list< ui::Command >::iterator i = m_shortcutCommands.begin(); i != m_shortcutCommands.end(); ++i)
	{
		int keyState;
		ui::VirtualKey virtualKey;

		if (!findShortcutCommandMapping(m_mergedSettings, i->getName(), keyState, virtualKey))
		{
#if defined(_DEBUG)
			log::info << L"No shortcut mapping for \"" << i->getName() << L"\"" << Endl;
#endif
			continue;
		}

		m_shortcutTable->addCommand(keyState, virtualKey, *i);
	}
}

void EditorForm::saveCurrentDocument()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// First iterate all object editor dialogs to see if focus is in any of those,
	// if so then we simulate an "Apply" in active one.
	for (Ref< Widget > child = getFirstChild(); child; child = child->getNextSibling())
	{
		Ref< ObjectEditorDialog > objectEditorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
		if (objectEditorDialog && objectEditorDialog->containFocus())
		{
			objectEditorDialog->apply(true);
			return;
		}
	}

	// Get active editor page and commit it's primary instance.
	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	if (tabPage)
	{
		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (!editorPage)
			return;

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (!document)
			return;

		ui::Command shouldSave(L"Editor.ShouldSave");
		editorPage->handleCommand(shouldSave);

		bool result = document->save();
		checkModified();

		if (result)
		{
			m_statusBar->setText(L"Document saved successfully");
			log::info << L"Document saved successfully" << Endl;

			buildAssetsForOpenedEditors();
		}
		else
		{
			ui::MessageBox::show(
				this,
				i18n::Text(L"ERROR_MESSAGE_UNABLE_TO_SAVE_DOCUMENT"),
				i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_DOCUMENT"),
				ui::MbOk | ui::MbIconExclamation
			);
		}
	}
}

void EditorForm::saveAllDocuments()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	bool allSuccessfull = true;
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT(tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (!editorPage)
			continue;

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		T_ASSERT(document);

		ui::Command shouldSave(L"Editor.ShouldSave");
		editorPage->handleCommand(shouldSave);

		allSuccessfull &= document->save();
	}

	if (allSuccessfull)
	{
		m_statusBar->setText(L"Document(s) saved successfully");
		log::info << L"Document(s) saved successfully" << Endl;
	}
	else
	{
		ui::MessageBox::show(
			this,
			i18n::Text(L"ERROR_MESSAGE_UNABLE_TO_SAVE_DOCUMENT"),
			i18n::Text(L"ERROR_TITLE_UNABLE_TO_SAVE_DOCUMENT"),
			ui::MbOk | ui::MbIconExclamation
		);
	}

	checkModified();
}

void EditorForm::closeCurrentEditor()
{
	if (!m_activeEditorPage)
		return;

	// Ask user when trying to close an editor which contains unsaved data.
	if (currentModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
			return;
	}

	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	T_ASSERT(tabPage);
	T_ASSERT(tabPage->getData(L"EDITORPAGE") == m_activeEditorPage);

	if (m_activeEditorPageSite)
		m_activeEditorPageSite->hide();

	m_activeEditorPage->destroy();

	m_activeEditorPageSite = nullptr;
	m_activeEditorPage = nullptr;

	m_activeDocument->close();
	m_activeDocument = nullptr;

	m_tab->removePage(tabPage);
	m_tab->update();

	tabPage->destroy();
	tabPage = nullptr;

	tabPage = m_tab->getActivePage();
	if (tabPage)
		setActiveEditorPage(tabPage->getData< IEditorPage >(L"EDITORPAGE"));
	else
		setActiveEditorPage(0);
}

void EditorForm::closeAllEditors()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Get all other pages to close; ignore home.
	RefArray< ui::TabPage > closePages;
	for (int32_t i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		if (tabPage->getData< IEditorPage >(L"EDITORPAGE") == nullptr)
			continue;

		closePages.push_back(tabPage);
	}

	// Close found pages.
	while (!closePages.empty())
	{
		Ref< ui::TabPage > tabPage = closePages.back();
		closePages.pop_back();

		T_ASSERT(tabPage);
		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (editorPage)
		{
			T_ASSERT(editorPage != m_activeEditorPage);
			editorPage->destroy();
		}

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (document)
			document->close();
	}

	m_tab->update();

	m_activeEditorPage = nullptr;
	m_activeEditorPageSite = nullptr;
	m_activeDocument = nullptr;

	setPropertyObject(nullptr);
}

void EditorForm::closeAllOtherEditors()
{
	T_ANONYMOUS_VAR(EnterLeave)(
		makeFunctor(this, &EditorForm::setCursor, ui::CrWait),
		makeFunctor(this, &EditorForm::resetCursor)
	);

	// Get all other pages to close; ignore active page and home.
	RefArray< ui::TabPage > closePages;
	for (int32_t i = 0; i < m_tab->getPageCount(); ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		if (tabPage == m_tab->getActivePage() || tabPage->getData< IEditorPage >(L"EDITORPAGE") == nullptr)
			continue;

		closePages.push_back(tabPage);
	}

	// Close found pages.
	while (!closePages.empty())
	{
		Ref< ui::TabPage > tabPage = closePages.back();
		closePages.pop_back();

		T_ASSERT(tabPage);
		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (editorPage)
		{
			T_ASSERT(editorPage != m_activeEditorPage);
			editorPage->destroy();
		}

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (document)
			document->close();
	}

	m_tab->update();
}

void EditorForm::findInDatabase()
{
	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	T_ASSERT(tabPage);

	Ref< db::Instance > instance = tabPage->getData< db::Instance >(L"PRIMARY");
	if (instance)
		highlightInstance(instance);
}

void EditorForm::activatePreviousEditor()
{
	Ref< ui::TabPage > previousTabPage = m_tab->cycleActivePage(false);
	if (previousTabPage)
	{
		Ref< IEditorPage > editorPage = previousTabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::activateNextEditor()
{
	Ref< ui::TabPage > nextTabPage = m_tab->cycleActivePage(true);
	if (nextTabPage)
	{
		Ref< IEditorPage > editorPage = nextTabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}
}

void EditorForm::loadModules()
{
#if !defined(T_STATIC)
	std::set< std::wstring > modulePaths = m_mergedSettings->getProperty< std::set< std::wstring > >(L"Editor.ModulePaths");
	std::set< std::wstring > modules = m_mergedSettings->getProperty< std::set< std::wstring > >(L"Editor.Modules");

	std::vector< Path > modulePathsFlatten(modulePaths.begin(), modulePaths.end());
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		Ref< Library > library = new Library();
		if (library->open(*i, modulePathsFlatten, true))
		{
			log::info << L"Module \"" << *i << L"\" loaded successfully" << Endl;
			library->detach();
		}
		else
			log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
	}
#endif
}

void EditorForm::loadLanguageDictionary()
{
	std::wstring dictionaryFile = m_mergedSettings->getProperty< std::wstring >(L"Editor.Dictionary", L"$(TRAKTOR_HOME)/resources/runtime/editor/locale/english/English.xml");

	Ref< IStream > file = FileSystem::getInstance().open(dictionaryFile, File::FmRead);
	if (!file)
	{
		log::warning << L"Unable to open dictionary \"" << dictionaryFile << L"\"; file missing." << Endl;
		return;
	}

	Ref< i18n::Dictionary > dictionary = dynamic_type_cast< i18n::Dictionary* >(xml::XmlDeserializer(file, dictionaryFile).readObject());
	file->close();

	if (dictionary)
		i18n::I18N::getInstance().appendDictionary(dictionary);
	else
		log::warning << L"Unable to load dictionary \"" << dictionaryFile << L"\"; possibly corrupted." << Endl;
}

void EditorForm::loadHelpDictionary()
{
	std::wstring helpFile = m_mergedSettings->getProperty< std::wstring >(L"Editor.Help");
	if (helpFile.empty())
		return;

	Ref< IStream > file = FileSystem::getInstance().open(helpFile, File::FmRead);
	if (!file)
	{
		log::warning << L"Unable to open dictionary \"" << helpFile << L"\"; file missing." << Endl;
		return;
	}

	Ref< i18n::Dictionary > dictionary = dynamic_type_cast< i18n::Dictionary* >(xml::XmlDeserializer(file, helpFile).readObject());
	file->close();

	if (dictionary)
		i18n::I18N::getInstance().appendDictionary(dictionary);
}

void EditorForm::checkModified()
{
	bool needUpdate = false;

	int pageCount = m_tab->getPageCount();
	for (int i = 0; i < pageCount; ++i)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(i);
		T_ASSERT(tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (!editorPage)
			continue;

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (!document)
			continue;

		// Add or remove asterix on tab.
		std::wstring tabName = tabPage->getText();
		if (document->modified())
		{
			if (tabName[tabName.length() - 1] != L'*')
			{
				tabPage->setText(tabName + L"*");
				needUpdate = true;
			}
		}
		else
		{
			if (tabName[tabName.length() - 1] == L'*')
			{
				tabPage->setText(tabName.substr(0, tabName.length() - 1));
				needUpdate = true;
			}
		}
	}

	if (needUpdate)
		m_tab->update();
}

bool EditorForm::currentModified()
{
	Ref< ui::TabPage > tabPage = m_tab->getActivePage();
	if (!tabPage)
		return false;

	checkModified();

	std::wstring tabName = tabPage->getText();
	return tabName[tabName.length() - 1] == L'*';
}

bool EditorForm::anyModified()
{
	checkModified();

	bool unsavedInstances = false;
	for (int i = 0; i < m_tab->getPageCount(); ++i)
	{
		std::wstring tabName = m_tab->getPage(i)->getText();
		if (tabName[tabName.length() - 1] == L'*')
		{
			unsavedInstances = true;
			break;
		}
	}

	return unsavedInstances;
}

bool EditorForm::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.NewWorkspace")
		createWorkspace();
	else if (command == L"Editor.OpenWorkspace")
		openWorkspace();
	else if (command == L"Editor.OpenRecentWorkspace")
	{
		Ref< Path > recentPath = dynamic_type_cast< Path* >(command.getData());
		if (recentPath)
			openWorkspace(*recentPath);
	}
	else if (command == L"Editor.Save")
		saveCurrentDocument();
	else if (command == L"Editor.SaveAll")
		saveAllDocuments();
	else if (command == L"Editor.CloseEditor")
		closeCurrentEditor();
	else if (command == L"Editor.CloseAllOtherEditors")
		closeAllOtherEditors();
	else if (command == L"Editor.FindInDatabase")
		findInDatabase();
	else if (command == L"Editor.Build")
		buildAssets(false);
	else if (command == L"Editor.Rebuild")
	{
		if (ui::MessageBox::show(this, i18n::Text(L"EDITOR_SURE_TO_REBUILD_MESSAGE"), i18n::Text(L"EDITOR_SURE_TO_REBUILD_CAPTION"), ui::MbYesNo | ui::MbIconExclamation) == ui::DrYes)
			buildAssets(true);
	}
	else if (command == L"Editor.CancelBuild")
		buildCancel();
	else if (command == L"Editor.ActivatePreviousEditor")
		activatePreviousEditor();
	else if (command == L"Editor.ActivateNextEditor")
		activateNextEditor();
	else if (command == L"Editor.Workspace")
	{
		if (!m_workspacePath.empty())
		{
			WorkspaceDialog workspaceDialog;
			if (workspaceDialog.create(this, m_workspaceSettings))
			{
				if (workspaceDialog.showModal() == ui::DrOk)
				{
					// Create merged settings.
					if (m_workspaceSettings)
					{
						m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
						T_ASSERT(m_mergedSettings);
					}
					else
						m_mergedSettings = m_globalSettings;

					// Save modified workspace.
					if (saveGlobalSettings(m_workspacePath, m_workspaceSettings))
					{
						// Re-open workspace.
						Path workspacePath = m_workspacePath;
						closeWorkspace();
						openWorkspace(workspacePath);
					}
				}
				workspaceDialog.destroy();
			}
		}
	}
	else if (command == L"Editor.Settings")
	{
		SettingsDialog settingsDialog;
		if (settingsDialog.create(this, m_originalSettings, m_globalSettings, m_shortcutCommands))
		{
			if (settingsDialog.showModal() == ui::DrOk)
			{
				// Create merged settings.
				if (m_workspaceSettings)
				{
					m_mergedSettings = m_globalSettings->merge(m_workspaceSettings, PropertyGroup::MmJoin);
					T_ASSERT(m_mergedSettings);
				}
				else
					m_mergedSettings = m_globalSettings;

				// Load editor stylesheet.
				Ref< ui::StyleSheet > styleSheetShared = loadStyleSheet(L"$(TRAKTOR_HOME)/resources/runtime/themes/Shared/StyleSheet.xss");
				T_FATAL_ASSERT(styleSheetShared);
				
				std::wstring styleSheetName = m_mergedSettings->getProperty< std::wstring >(L"Editor.StyleSheet", L"$(TRAKTOR_HOME)/resources/runtime/themes/Light.xss");
				Ref< ui::StyleSheet > styleSheet = loadStyleSheet(styleSheetName);
				if (styleSheet)
					ui::Application::getInstance()->setStyleSheet(
						styleSheetShared->merge(styleSheet)
					);
				else
					log::error << L"Unable to load stylesheet " << styleSheetName << Endl;

				// Save modified settings; do this here as well as at termination
				// as we want to make sure changes doesn't get lost in case of a crash.
				Ref< const PropertyGroup > userSettings = m_originalSettings->difference(m_globalSettings);
				if (saveUserSettings(m_settingsPath, userSettings))
				{
					updateShortcutTable();
					for (int i = 0; i < m_tab->getPageCount(); ++i)
					{
						Ref< ui::TabPage > tabPage = m_tab->getPage(i);
						Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
						if (editorPage)
							editorPage->handleCommand(ui::Command(L"Editor.SettingsChanged"));
					}
					m_propertiesView->handleCommand(ui::Command(L"Editor.SettingsChanged"));
				}

				update();
			}

			settingsDialog.destroy();
		}
	}
	else if (command == L"Editor.ViewHome")
	{
		std::wstring url = m_mergedSettings->getProperty< std::wstring >(L"Editor.HomeUrl", L"");
		if (!url.empty())
		{
			Ref< ui::TabPage > tabPage = new ui::TabPage();
			tabPage->create(m_tab, i18n::Text(L"EDITOR_HOME"), 0, new ui::FloodLayout());

			Ref< WebBrowserPage > homePage = new WebBrowserPage(this);
			homePage->create(tabPage, url);

			m_tab->addPage(tabPage);
			m_tab->update(0, true);
		}
	}
	else if (command == L"Editor.ViewDatabase")
	{
		m_dataBaseView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewProperties")
	{
		m_propertiesView->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewLog")
	{
		m_tabOutput->show();
		m_dock->update();
	}
	else if (command == L"Editor.ViewOther")
	{
		Ref< ui::Widget > panelWidget = checked_type_cast< ui::Widget* >(command.getData());
		if (panelWidget)
			showAdditionalPanel(panelWidget);
	}
	else if (command == L"Editor.QuickOpen")
	{
		if (m_sourceDatabase)
		{
			QuickOpenDialog quickOpenDlg(this);
			if (quickOpenDlg.create(this))
			{
				Ref< db::Instance > instance = quickOpenDlg.showDialog();
				quickOpenDlg.destroy();
				if (instance)
					openEditor(instance);
			}
		}
	}
	else if (command == L"Editor.Exit")
		ui::Application::getInstance()->exit(0);
	else if ((command.getFlags() & ui::Command::CfId) == ui::Command::CfId)
	{
		Ref< IEditorTool > tool = m_editorTools[command.getId()];
		T_ASSERT(tool);

		// Issue a build if resources need to be up-to-date.
		std::set< Guid > dependencies;
		if (tool->needOutputResources(dependencies))
		{
			buildAssets(std::vector< Guid >(dependencies.begin(), dependencies.end()), false);
			buildWaitUntilFinished();
		}

		if (tool->launch(this, this, nullptr))
			m_dataBaseView->updateView();
		else
			result = false;
	}
	else
	{
		result = false;

		// Propagate command to database view; if it contains focus.
		if (!result)
		{
			if (m_dataBaseView->containFocus())
				result = m_dataBaseView->handleCommand(command);
		}

		// Propagate command to properties view; if it contains focus.
		if (!result)
		{
			if (m_propertiesView->containFocus())
				result = m_propertiesView->handleCommand(command);
		}

		// Propagate command to active editor; if it contains focus.
		if (!result)
		{
			bool activeEditorFocus = false;

			Ref< ui::TabPage > tabPage = m_tab->getActivePage();
			if (tabPage && tabPage->containFocus())
				activeEditorFocus = true;

			if (!activeEditorFocus && m_activeEditorPageSite)
			{
				const auto& panelWidgets = m_activeEditorPageSite->getPanelWidgets();
				for (auto i = panelWidgets.begin(); i != panelWidgets.end(); ++i)
				{
					if (i->first && i->first->containFocus())
					{
						activeEditorFocus = true;
						break;
					}
				}
			}

			if (activeEditorFocus)
			{
				if (m_activeEditorPage)
					result = m_activeEditorPage->handleCommand(command);
			}
		}

		if (!result)
		{
			for (Ref< Widget > child = getFirstChild(); child; child = child->getNextSibling())
			{
				ObjectEditorDialog* editorDialog = dynamic_type_cast< ObjectEditorDialog* >(child);
				if (editorDialog && editorDialog->containFocus())
				{
					result = editorDialog->handleCommand(command);
					break;
				}
			}
		}
	}

	// Propagate commands to plugins; even if it's already consumed.
	for (RefArray< EditorPluginSite >::iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
		result |= (*i)->handleCommand(command, result);

	return result;
}

void EditorForm::eventShortcut(ui::ShortcutEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventMenuClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventToolClicked(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (handleCommand(command))
		event->consume();
}

void EditorForm::eventTabButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		if (m_tab->getPageAt(event->getPosition()) != nullptr)
		{
			const ui::MenuItem* selectedItem = m_menuTab->showModal(m_tab, event->getPosition());
			if (selectedItem)
				handleCommand(selectedItem->getCommand());
		}
	}
}

void EditorForm::eventTabSelChange(ui::TabSelectionChangeEvent* event)
{
	Ref< ui::TabPage > tabPage = event->getTabPage();
	Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
	setActiveEditorPage(editorPage);
}

void EditorForm::eventTabClose(ui::TabCloseEvent* event)
{
	Ref< ui::TabPage > tabPage = event->getTabPage();

	// Ask user when trying to close an editor which contains unsaved data.
	if (currentModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCE_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
		{
			event->consume();
			event->cancel();
			return;
		}
	}

	m_tab->removePage(tabPage);

	Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
	if (editorPage)
	{
		T_ASSERT(m_activeEditorPage == editorPage);
		editorPage->destroy();
		editorPage = nullptr;
		m_activeEditorPage = nullptr;
	}

	Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
	if (document)
	{
		T_ASSERT(m_activeDocument == document);
		document->close();
		document = nullptr;
		m_activeDocument = nullptr;
	}

	tabPage->destroy();
	tabPage = nullptr;

	setPropertyObject(0);

	tabPage = m_tab->getActivePage();
	if (tabPage)
	{
		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		setActiveEditorPage(editorPage);
	}

	m_tab->update();
}

void EditorForm::eventClose(ui::CloseEvent* event)
{
	if (anyModified())
	{
		int result = ui::MessageBox::show(
			this,
			i18n::Text(L"QUERY_MESSAGE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			i18n::Text(L"QUERY_TITLE_INSTANCES_NOT_SAVED_CLOSE_EDITOR"),
			ui::MbIconExclamation | ui::MbYesNo
		);
		if (result == ui::DrNo)
		{
			event->consume();
			event->cancel();
			return;
		}
	}

	while (m_tab->getPageCount() > 0)
	{
		Ref< ui::TabPage > tabPage = m_tab->getPage(0);
		m_tab->removePage(tabPage);

		Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
		if (editorPage)
		{
			editorPage->destroy();
			editorPage = nullptr;
		}

		Ref< Document > document = tabPage->getData< Document >(L"DOCUMENT");
		if (document)
		{
			document->close();
			document = nullptr;
		}
	}

	// Save panes visible.
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.DatabaseVisible", m_dataBaseView->isVisible(false));
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.PropertiesVisible", m_propertiesView->isVisible(false));
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.LogVisible", m_logView->isVisible(false));

	// Save form placement.
	ui::Rect rc = getNormalRect();
	m_globalSettings->setProperty< PropertyBoolean >(L"Editor.Maximized", isMaximized());
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PositionX", rc.left);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.PositionY", rc.top);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.SizeWidth", rc.getWidth());
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.SizeHeight", rc.getHeight());

	// Save desktop size.
	const auto desktopSize = getDesktopSizeEstimate();
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.LastDesktopWidth", desktopSize.cx);
	m_globalSettings->setProperty< PropertyInteger >(L"Editor.LastDesktopHeight", desktopSize.cy);

	// Save settings; generate a diff patch to simplify adding new properties to original settings.
	Ref< const PropertyGroup > userSettings = m_originalSettings->difference(m_globalSettings);
	saveUserSettings(m_settingsPath, userSettings);

	ui::Application::getInstance()->exit(0);
}

void EditorForm::eventTimer(ui::TimerEvent* /*event*/)
{
	Ref< const db::IEvent > event;
	bool remote;
	bool updateView = false;
	bool building = (bool)(m_threadBuild && !m_threadBuild->finished());

	// Only check for database modifications when we're not building,
	// as building causes a lot of db traffic we wait until it's finished.
	if (!building)
	{
		std::vector< std::pair< db::Database*, Guid > > eventIds;

		// Check if there is any committed instances into
		// source database.
		if (m_sourceDatabase)
		{
			bool anyCommitted = false;
			while (m_sourceDatabase->getEvent(event, remote))
			{
				auto committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event);
				if (committed)
				{
					log::debug << (remote ? L"Remotely" : L"Locally") << L" modified source instance " << committed->getInstanceGuid().format() << L" detected; propagate to editor pages..." << Endl;
					eventIds.push_back(std::make_pair(m_sourceDatabase, committed->getInstanceGuid()));
					anyCommitted = true;
				}
				updateView |= remote;
			}
			if (anyCommitted && m_mergedSettings->getProperty< bool >(L"Editor.BuildWhenSourceModified"))
			{
				buildAssetsForOpenedEditors();

				// Notify all plugins of automatic build.
				for (auto editorPluginSite : m_editorPluginSites)
					editorPluginSite->handleCommand(ui::Command(L"Editor.AutoBuild"), false);
			}
		}

		// Gather events from output database; used to notify
		// editors what to reload.
		if (m_outputDatabase)
		{
			while (m_outputDatabase->getEvent(event, remote))
			{
				const db::EvtInstanceCommitted* committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event);
				if (committed)
				{
					log::debug << (remote ? L"Remotely" : L"Locally") << L" modified output instance " << committed->getInstanceGuid().format() << L" detected; propagate to editor pages..." << Endl;
					eventIds.push_back(std::make_pair(m_outputDatabase, committed->getInstanceGuid()));
				}
			}
		}

		// Only propagate events when build is finished.
		if (
			!eventIds.empty() &&
			m_lockBuild.wait(0)
		)
		{
			// Propagate database event to editor pages in order for them to flush resources.
			for (int i = 0; i < m_tab->getPageCount(); ++i)
			{
				Ref< ui::TabPage > tabPage = m_tab->getPage(i);
				Ref< IEditorPage > editorPage = tabPage->getData< IEditorPage >(L"EDITORPAGE");
				if (editorPage)
				{
					for (auto eventId : eventIds)
						editorPage->handleDatabaseEvent(eventId.first, eventId.second);
				}
			}

			// Propagate database event to object editor dialogs.
			for (ui::Widget* child = this->getFirstChild(); child; child = child->getNextSibling())
			{
				if (auto objectEditor = dynamic_type_cast< ObjectEditorDialog* >(child))
				{
					for (auto eventId : eventIds)
						objectEditor->handleDatabaseEvent(eventId.first, eventId.second);
				}
			}

			// Propagate database event to editor plugins.
			for (auto editorPluginSite : m_editorPluginSites)
			{
				for (auto eventId : eventIds)
					editorPluginSite->handleDatabaseEvent(eventId.first, eventId.second);
			}

			m_lockBuild.release();
			log::debug << eventIds.size() << L" database change(s) notified." << Endl;
		}
	}

	// We need to update database view as another process has modified database.
	if (updateView)
		m_dataBaseView->updateView();

	// Update modified flags.
	checkModified();

	// Hide build progress if build thread has finished.
	if (!building)
	{
		m_buildProgress->setVisible(false);
		m_statusBar->setText(i18n::Text(L"STATUS_IDLE"));
	}
	else
		m_buildProgress->setVisible(true);
}

void EditorForm::threadAssetMonitor()
{
	while (!m_threadAssetMonitor->stopped())
	{
		if (
			m_sourceDatabase &&
			m_mergedSettings->getProperty< bool >(L"Editor.BuildWhenAssetModified") &&
			m_lockBuild.wait(0)
		)
		{
			RefArray< db::Instance > assetInstances;
			db::recursiveFindChildInstances(
				m_sourceDatabase->getRootGroup(),
				db::FindInstanceByType(type_of< Asset >()),
				assetInstances
			);

			if (!assetInstances.empty())
			{
				std::vector< Guid > modifiedAssets;
				RefArray< const File > modifiedFiles;

				std::wstring assetPath = m_mergedSettings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

				for (RefArray< db::Instance >::const_iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
				{
					Ref< Asset > asset = (*i)->getObject< Asset >();
					if (!asset)
						continue;

					Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

					RefArray< File > files;
					FileSystem::getInstance().find(fileName, files);

					for (RefArray< File >::const_iterator j = files.begin(); j != files.end(); ++j)
					{
						const File* file = *j;
						uint32_t flags = file->getFlags();
						if ((flags & File::FfArchive) == File::FfArchive)
						{
							log::info << L"Source asset \"" << file->getPath().getPathName() << L"\" modified" << Endl;
							modifiedFiles.push_back(file);
							modifiedAssets.push_back((*i)->getGuid());
						}
					}
				}

				for (RefArray< const File >::const_iterator i = modifiedFiles.begin(); i != modifiedFiles.end(); ++i)
				{
					const File* file = *i;
					FileSystem::getInstance().modify(file->getPath(), file->getFlags() & ~File::FfArchive);
				}

				m_lockBuild.release();

				if (!modifiedAssets.empty())
				{
					log::info << L"Modified source asset(s) detected; building asset(s)..." << Endl;
					buildAssets(modifiedAssets, false);

					// Notify all plugins of automatic build.
					for (RefArray< EditorPluginSite >::const_iterator i = m_editorPluginSites.begin(); i != m_editorPluginSites.end(); ++i)
						(*i)->handleCommand(ui::Command(L"Editor.AutoBuild"), false);
				}
			}
		}

		m_threadAssetMonitor->sleep(1000);
	}
}

void EditorForm::threadOpenWorkspace(const Path& workspacePath, int32_t& progress)
{
	Ref< PropertyGroup > workspaceSettings;

	if (!loadSettings(workspacePath, workspaceSettings, 0))
	{
		log::error << L"Failed to load workspace; load failed." << Endl;
		return;
	}
	T_FATAL_ASSERT (workspaceSettings != nullptr)

	progress = 100;

	// Change working directory to workspace file.
	FileSystem::getInstance().setCurrentVolumeAndDirectory(workspacePath.getPathOnly());

	// Create merged settings.
	Ref< PropertyGroup > mergedSettings = m_globalSettings->merge(workspaceSettings, PropertyGroup::MmJoin);
	T_FATAL_ASSERT (mergedSettings != nullptr);

	progress = 200;

	// Open databases.
	std::wstring sourceDatabaseCs = mergedSettings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	Ref< db::Database > sourceDatabase = openDatabase(sourceDatabaseCs, false);
	if (!sourceDatabase)
	{
		log::error << L"Unable to open source database \"" << sourceDatabaseCs << L"\"" << Endl;
		return;
	}

	progress = 600;

	std::wstring outputDatabaseCs = mergedSettings->getProperty< std::wstring >(L"Editor.OutputDatabase");
	Ref< db::Database > outputDatabase = openDatabase(outputDatabaseCs, true);
	if (!outputDatabase)
	{
		log::error << L"Unable to open output database \"" << outputDatabaseCs << L"\"" << Endl;
		return;
	}

	// Successfully opened workspace.
	m_workspaceSettings = workspaceSettings;
	m_mergedSettings = mergedSettings;
	m_sourceDatabase = sourceDatabase;
	m_outputDatabase = outputDatabase;

	progress = 1000;
}

	}
}
