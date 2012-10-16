#include "Amalgam/App/Win32/ErrorDialog.h"
#include "Amalgam/Impl/Application.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Ui/Application.h"
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/WidgetFactoryWin32.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

namespace
{

class LogTailTarget : public ILogTarget
{
public:
	Semaphore m_lock;
	std::list< std::wstring > m_tail;

	virtual void log(const std::wstring& str)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (m_tail.size() > 100)
			m_tail.pop_front();
		m_tail.push_back(str);
	}
};

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(const std::wstring& str)
	{
		(*m_stream) << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

class LogDualTarget : public ILogTarget
{
public:
	LogDualTarget(ILogTarget* target1, ILogTarget* target2)
	:	m_target1(target1)
	,	m_target2(target2)
	{
	}

	virtual void log(const std::wstring& str)
	{
		m_target1->log(str);
		m_target2->log(str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

bool saveSettings(const PropertyGroup* settings, const Path& settingsFile)
{
	T_ASSERT (settings);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmWrite);
	if (!file)
		return false;

	bool result = xml::XmlSerializer(file).writeObject(settings);
	file->close();
	
	return result;
}

void showErrorDialog(const std::list< std::wstring >& tail)
{
	amalgam::ErrorDialog errorDialog;
	if (errorDialog.create())
	{
		for (std::list< std::wstring >::const_iterator i = tail.begin(); i != tail.end(); ++i)
			errorDialog.addErrorString(*i);
			
		errorDialog.addErrorString(L"Please copy this information and contact");
		errorDialog.addErrorString(L"support@doctorentertainment.com");

		errorDialog.showModal();
		errorDialog.destroy();
	}
}

bool checkPreconditions()
{
	BOOL sseSupported = ::IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
	if (!sseSupported)
	{
		log::error << L"This game requires a CPU with SSE support" << Endl;
		return false;
	}
	return true;
}

}

#if !defined(WINCE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPWSTR szCmdLine, int)
#endif
{
	std::vector< std::wstring > argv;

	wchar_t szFilename[MAX_PATH] = L"";
	GetModuleFileName(NULL, szFilename, sizeof(szFilename));
	argv.push_back(szFilename);

#if !defined(WINCE)
	Split< std::wstring >::any(mbstows(szCmdLine), L" \t", argv);
#else
	Split< std::wstring >::any(szCmdLine, L" \t", argv);
#endif
	CommandLine cmdLine(argv);
	Ref< traktor::IStream > logFile;

	if (!checkPreconditions())
		return 1;

	std::wstring writablePath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB";
	FileSystem::getInstance().makeAllDirectories(writablePath);

#if !defined(_DEBUG)
	if (!IsDebuggerPresent())
	{
		logFile = FileSystem::getInstance().open(L"Application.log", File::FmWrite);
		if (logFile)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logTarget = new LogStreamTarget(logStream);

			log::info   .setTarget(logTarget);
			log::warning.setTarget(logTarget);
			log::error  .setTarget(logTarget);

			log::info << L"Log file \"Application.log\" created" << Endl;
		}
		else
			log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}
#endif

	Ref< LogTailTarget > logTail = new LogTailTarget();
	log::info   .setTarget(new LogDualTarget(logTail, log::info   .getTarget()));
	log::warning.setTarget(new LogDualTarget(logTail, log::warning.getTarget()));
	log::error  .setTarget(new LogDualTarget(logTail, log::error  .getTarget()));

	// Initialize native UI.
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32()
	);

	try
	{
		// Override settings path either from command line or application bundle.
		Path settingsPath = L"Application.config";
		if (cmdLine.getCount() >= 1)
			settingsPath = cmdLine.getString(0);

		Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
		if (!defaultSettings)
		{
			log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
			log::error << L"Please reinstall application." << Endl;
			showErrorDialog(logTail->m_tail);
			return 1;
		}

		Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
		T_FATAL_ASSERT (settings);

		// Merge user settings into application settings.
		if (!cmdLine.hasOption('s', L"no-settings"))
		{
			Path userSettingsPath;
			Ref< PropertyGroup > userSettings;

			// First try to load user settings from current working directory; ie. same directory as
			// the main executable.
			userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			userSettings = loadSettings(userSettingsPath);
	
			// Try to load user settings from user's application data path; sometimes it's not possible
			// to store user settings alongside executable due to restrictive privileges.
			if (!userSettings)
			{
				userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
				userSettings = loadSettings(userSettingsPath);
			}

			if (userSettings)
				settings = settings->mergeReplace(userSettings);
		}

		if (!settings)
		{
			log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
			log::error << L"Please reinstall application." << Endl;
			showErrorDialog(logTail->m_tail);
			return 1;
		}

		// Create amalgam application.
		Ref< amalgam::Application > application = new amalgam::Application();
		if (application->create(
			defaultSettings,
			settings,
			0
		))
		{
			for (;;)
			{
				if (!application->update())
					break;
			}

			safeDestroy(application);

			// Save user settings.
			if (!cmdLine.hasOption('s', L"no-settings"))
			{
				Path userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
				if (!saveSettings(settings, userSettingsPath))
				{
					userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
					if (!saveSettings(settings, userSettingsPath))
						log::error << L"Unable to save user settings; user changes not saved" << Endl;
				}
			}
		}
		else
		{
			safeDestroy(application);
			showErrorDialog(logTail->m_tail);
		}
	}
	catch (...)
	{
		log::error << L"Unhandled exception occurred" << Endl;
		showErrorDialog(logTail->m_tail);
	}

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	if (logFile)
	{
		log::info   .setTarget(0);
		log::warning.setTarget(0);
		log::error  .setTarget(0);

		logFile->close();
		logFile;
	}
#endif

	return 0;
}
