#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"
#include "Net/Network.h"
#include "Run/App/ProduceOutput.h"
#include "Run/App/Run.h"
#include "Run/App/StdOutput.h"
#include "Script/IScriptContext.h"
#include "Script/Lua/ScriptCompilerLua.h"
#include "Script/Lua/ScriptManagerLua.h"

namespace
{

traktor::Ref< traktor::script::IScriptCompiler > g_scriptCompiler;
traktor::Ref< traktor::script::IScriptManager > g_scriptManager;

}

namespace traktor
{
	namespace run
	{

/*! Execute run script. */
int32_t executeRun(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	// Compile script into a runnable blob.
	Ref< script::IScriptBlob > scriptBlob = g_scriptCompiler->compile(fileName.getPathName(), text, nullptr);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create script context..
	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 1;

	// Expose some environment variables of running script.
	OS::getInstance().setEnvironment(L"RUN_SCRIPT", fileName.getPathName());
	OS::getInstance().setEnvironment(L"RUN_SCRIPT_PATH", fileName.getPathOnly());

	// Setup globals in script context.
	scriptContext->setGlobal("environment", Any::fromObject(OS::getInstance().getEnvironment()));
	scriptContext->setGlobal("run", Any::fromObject(new Run(g_scriptCompiler, g_scriptManager, scriptContext)));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	scriptContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	scriptContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));

	// Load script into context.
	scriptContext->load(scriptBlob);

	// Execute optional "main" function.
	Any retval = Any::fromInt32(0);
	if (scriptContext->haveFunction("main"))
	{
		// Transform arguments into script array.
		std::vector< std::wstring > args;
		for (int32_t i = 1; i < cmdLine.getCount(); ++i)
			args.push_back(cmdLine.getString(i));

		Any a = CastAny< std::vector< std::wstring > >::set(args);
		retval = scriptContext->executeFunction("main", 1, &a);
	}

	safeDestroy(scriptContext);

	return retval.getInt32();
}

/*! Execute template script. */
int32_t executeTemplate(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	Ref< ProduceOutput > o = new ProduceOutput();

	// Transform into valid script.
	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		size_t s = text.find(L"<!--", offset);
		if (s == text.npos)
			break;

		size_t e = text.find(L"--!>", s);
		if (e == text.npos)
		{
			log::error << L"Template syntax error; missing end" << Endl;
			return 1;
		}

		int32_t id = o->addSection(text.substr(offset, s - offset));
		ss << L"output:printSection(" << id << L")" << Endl;
		ss << text.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(text.substr(offset));
	ss << L"output:printSection(" << id << L")" << Endl;

	Ref< script::IScriptBlob > scriptBlob = g_scriptCompiler->compile(fileName.getPathName(), ss.str(), nullptr);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Transform arguments into script array.
	std::vector< std::wstring > args;
	for (int32_t i = 1; i < cmdLine.getCount(); ++i)
		args.push_back(cmdLine.getString(i));

	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 1;

	// Expose some environment variables of running script.
	OS::getInstance().setEnvironment(L"RUN_SCRIPT", fileName.getPathName());
	OS::getInstance().setEnvironment(L"RUN_SCRIPT_PATH", fileName.getPathOnly());

	// Setup globals in script context.
	scriptContext->setGlobal("environment", Any::fromObject(OS::getInstance().getEnvironment()));
	scriptContext->setGlobal("run", Any::fromObject(new Run(g_scriptCompiler, g_scriptManager, scriptContext)));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	scriptContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	scriptContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));
	scriptContext->setGlobal("output", Any::fromObject(o));
	scriptContext->setGlobal("args", CastAny< std::vector< std::wstring > >::set(args));

	scriptContext->load(scriptBlob);

	safeDestroy(scriptContext);

	log::info << o->getProduct() << Endl;
	return 0;
}

	}
}

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Traktor.Run.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << Endl;
		log::info << L"Usage: Run (option(s)) [<file>.run|<file>.template] (args ...)" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -as-run                   Run file as run" << Endl;
		log::info << L"    -as-template              Run file as template" << Endl;
		log::info << Endl;
		log::info << L"  For .run files:" << Endl;
		log::info << L"    -e,-entry-point=function  Script entry point (default \"main\")" << Endl;
		log::info << Endl;
		log::info << L"  For .template files:" << Endl;
		log::info << L"    -o,-output=filename       Output file (default stdout)" << Endl;
		return 0;
	}

	Path fileName = cmdLine.getString(0);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
	{
		log::error << L"Failed to open \"" << fileName.getPathName() << L"\"." << Endl;
		return 1;
	}

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	safeClose(file);

	std::wstring text = ss.str();

	g_scriptCompiler = new script::ScriptCompilerLua();
	g_scriptManager = new script::ScriptManagerLua();

	run::Run::registerRuntimeClasses(g_scriptManager);
	net::Network::initialize();

	int32_t result = 1;

	bool explicitRun = cmdLine.hasOption(L"as-run");
	bool explicitTemplate = cmdLine.hasOption(L"as-template");

	if ((explicitRun && !explicitTemplate) || compareIgnoreCase(fileName.getExtension(), L"run") == 0)
		result = run::executeRun(text, fileName, cmdLine);
	else if ((!explicitRun && explicitTemplate) || compareIgnoreCase(fileName.getExtension(), L"template") == 0)
		result = run::executeTemplate(text, fileName, cmdLine);
	else
		log::error << L"Unknown file type \"" << fileName.getExtension() << L"\"; must be either \"run\" or \"template\"." << Endl;

	net::Network::finalize();

	safeDestroy(g_scriptManager);
	return result;
}
