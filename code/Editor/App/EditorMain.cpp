#if defined(_WIN32)
#	include <Windows.h>
#endif
#if defined(_DEBUG)
#	include "Core/CycleRefDebugger.h"
#endif
#include "Core/Debug/Debugger.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/Splash.h"
#include "Net/Network.h"
#include "Ui/Application.h"

#if defined(_WIN32)
#	include "Ui/Win32/WidgetFactoryWin32.h"
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include "Ui/Cocoa/WidgetFactoryCocoa.h"
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Ui/X11/WidgetFactoryX11.h"
typedef traktor::ui::WidgetFactoryX11 WidgetFactoryImpl;
#endif

#if defined(__LINUX__)
#	include <X11/Xlib.h>
#endif

using namespace traktor;

#if defined(_WIN32)
// NVidia hack to get Optimus to enable NVidia GPU when possible.
extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

namespace
{

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		(*m_stream) << L"[" << DateTime::now().format(L"%H:%M:%S") << L"] " << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

}

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	wchar_t file[MAX_PATH] = L"";
	GetModuleFileName(NULL, file, sizeof_array(file));
	CommandLine cmdLine(file, mbstows(szCmdLine));
#endif

#if defined(__LINUX__) || defined(__RPI__)
	// Initialize X11 thread primitives; thus must be performed very early.
	XInitThreads();
#endif

#if defined(_DEBUG)
	//CycleRefDebugger cycleDebugger;
	//Object::setReferenceDebugger(&cycleDebugger);
#endif

	Ref< traktor::IStream > logFile;
#if !defined(_DEBUG)
	if (!Debugger::getInstance().isDebuggerAttached())
	{
		RefArray< File > logs;
		FileSystem::getInstance().find(L"Editor_*.log", logs);

		// Get "alive" log ids.
		std::vector< int32_t > logIds;
		for (auto log : logs)
		{
			std::wstring logName = log->getPath().getFileNameNoExtension();
			size_t p = logName.find(L'_');
			if (p != logName.npos)
			{
				int32_t id = parseString< int32_t >(logName.substr(p + 1), -1);
				if (id != -1)
					logIds.push_back(id);
			}
		}

		int32_t nextLogId = 0;
		if (!logIds.empty())
		{
			std::sort(logIds.begin(), logIds.end());

			// Don't keep more than 10 log files.
			while (logIds.size() >= 10)
			{
				StringOutputStream ss;
				ss << L"Editor_" << logIds.front() << L".log";
				FileSystem::getInstance().remove(ss.str());
				logIds.erase(logIds.begin());
			}

			nextLogId = logIds.back() + 1;
		}

		// Create new log file.
		StringOutputStream ss;
		ss << L"Editor_" << nextLogId << L".log";
		logFile = FileSystem::getInstance().open(ss.str(), File::FmWrite);
		if (logFile)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logTarget = new LogStreamTarget(logStream);

			log::info   .setGlobalTarget(logTarget);
			log::warning.setGlobalTarget(logTarget);
			log::error  .setGlobalTarget(logTarget);

			log::info << L"Log file \"" << ss.str() << L"\" created." << Endl;
		}
		else
			log::error << L"Unable to create log file; logging only to std pipes." << Endl;
	}
#endif

	// Check if environment is already set, else set to current working directory.
	std::wstring home;
	if (!OS::getInstance().getEnvironment(L"TRAKTOR_HOME", home))
	{
		Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
#if !defined(_WIN32)
		home = cwd.getPathNameNoExtension();
#else
		home = cwd.getPathName();
#endif
		OS::getInstance().setEnvironment(L"TRAKTOR_HOME", home);
	}

#if defined(__LINUX__) || defined(__RPI__)
	std::wstring writableFolder = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB";
	FileSystem::getInstance().makeAllDirectories(writableFolder);
#endif

	ui::Application::getInstance()->initialize(
		new WidgetFactoryImpl(),
		nullptr
	);

	net::Network::initialize();

	try
	{
#if !defined(_DEBUG)
		Ref< editor::Splash > splash;
		if (!cmdLine.hasOption(L"no-splash"))
		{
			splash = new editor::Splash();
			splash->create();

			for (int32_t i = 0; i < 10; ++i)
				ui::Application::getInstance()->process();
		}
#endif

		Ref< editor::EditorForm > editorForm = new editor::EditorForm();
		if (editorForm->create(cmdLine))
		{
#if !defined(_DEBUG)
			if (splash)
				splash->hide();
#endif

			ui::Application::getInstance()->execute();
			editorForm->destroy();
		}

#if !defined(_DEBUG)
		safeDestroy(splash);
#endif
	}
	catch (...)
	{
		traktor::log::error << L"Unhandled exception, application terminated." << Endl;
	}

	net::Network::finalize();

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	safeClose(logFile);
#endif

#if 0
	Object::setReferenceDebugger(0);
#endif
	return 0;
}
