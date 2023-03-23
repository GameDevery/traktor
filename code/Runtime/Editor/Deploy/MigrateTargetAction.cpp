/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <list>
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/Deploy/MigrateTargetAction.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/System/ResolveEnv.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor::runtime
{
	namespace
	{

struct FeaturePriorityPred
{
	bool operator () (const Feature* l, const Feature* r) const
	{
		return l->getPriority() < r->getPriority();
	}
};

std::wstring implodePropertyValue(const IPropertyValue* value)
{
	if (const PropertyString* valueString = dynamic_type_cast< const PropertyString* >(value))
		return PropertyString::get(valueString);
	else if (const PropertyStringArray* valueStringArray = dynamic_type_cast< const PropertyStringArray* >(value))
	{
		auto ss = PropertyStringArray::get(valueStringArray);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else if (const PropertyStringSet* valueStringSet = dynamic_type_cast< const PropertyStringSet* >(value))
	{
		auto ss = PropertyStringSet::get(valueStringSet);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else
		return L"";
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.MigrateTargetAction", MigrateTargetAction, ITargetAction)

MigrateTargetAction::MigrateTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const std::wstring& targetName,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& deployHost,
	const std::wstring& outputPath
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_targetName(targetName)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_deployHost(deployHost)
,	m_outputPath(outputPath)
{
}

bool MigrateTargetAction::execute(IProgressListener* progressListener)
{
	Ref< PropertyGroup > deploy = new PropertyGroup();
	std::wstring executableFile;

	// Get platform description object from database.
	Ref< Platform > platform = m_database->getObjectReadOnly< Platform >(m_targetConfiguration->getPlatform());
	if (!platform)
	{
		log::error << L"Unable to get platform object" << Endl;
		return false;
	}

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 100);

	// Ensure output directory exists.
	if (!FileSystem::getInstance().makeAllDirectories(m_outputPath))
	{
		log::error << L"Unable to create output path \"" << m_outputPath << L"\"" << Endl;
		return false;
	}

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=db;binary=true;journal=false");
	db::ConnectionString outputDatabaseCs(L"provider=traktor.db.CompactDatabase;fileName=Content.compact;flushAlways=false");
	db::ConnectionString applicationDatabaseCs(L"provider=traktor.db.CompactDatabase;fileName=Content.compact;readOnly=true");

	// Create migration configuration.
	Ref< PropertyGroup > migrateConfiguration = new PropertyGroup();

	// Get features; sorted by priority.
	const std::list< Guid >& featureIds = m_targetConfiguration->getFeatures();

	RefArray< const Feature > features;
	for (auto featureId : featureIds)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(featureId);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << featureId.format() << L"\"; feature skipped." << Endl;
			continue;
		}
		features.push_back(feature);
	}

	features.sort(FeaturePriorityPred());

	// Insert target's features into migrate configuration.
	for (auto feature : features)
	{
		Ref< const PropertyGroup > migrateProperties = feature->getMigrateProperties();
		if (!migrateProperties)
			continue;

		migrateConfiguration = migrateConfiguration->merge(migrateProperties, PropertyGroup::MmJoin);
	}

	migrateConfiguration->setProperty< PropertyString >(L"Migrate.SourceDatabase", sourceDatabaseCs.format());
	migrateConfiguration->setProperty< PropertyString >(L"Migrate.OutputDatabase", outputDatabaseCs.format());

	// Create target application configuration.
	Ref< PropertyGroup > applicationConfiguration = new PropertyGroup();

	// Insert features into runtime configuration. Also get executable file.
	for (auto feature : features)
	{
		Ref< const PropertyGroup > runtimeProperties = feature->getRuntimeProperties();
		if (runtimeProperties)
			applicationConfiguration = applicationConfiguration->merge(runtimeProperties, PropertyGroup::MmJoin);

		const Feature::Platform* fp = feature->getPlatform(m_targetConfiguration->getPlatform());
		if (fp)
		{
			if (fp->deploy)
				deploy = deploy->merge(fp->deploy, PropertyGroup::MmJoin);
			if (!fp->executableFile.empty())
				executableFile = fp->executableFile;
		}
		else
			log::warning << L"Feature \"" << feature->getDescription() << L"\" doesn't support selected platform." << Endl;
	}

	// Modify configuration to connect to migrated database.
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Database", applicationDatabaseCs.format());

	// Append target guid;s to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Root", m_targetConfiguration->getRoot().format());
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Startup", m_targetConfiguration->getStartup().format());
	applicationConfiguration->setProperty< PropertyString >(L"Input.Default", m_targetConfiguration->getDefaultInput().format());
	applicationConfiguration->setProperty< PropertyString >(L"Online.Config", m_targetConfiguration->getOnlineConfig().format());

	// Append application title.
	applicationConfiguration->setProperty< PropertyString >(L"Render.Title", m_targetName);

	// Write generated configurations in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
		m_outputPath + L"/Migrate.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(migrateConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write migrate configuration" << Endl;
		return false;
	}

	file = FileSystem::getInstance().open(
		m_outputPath + L"/Application.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(applicationConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write application configuration" << Endl;
		return false;
	}

	// Get list of used modules from application configuration.
	auto runtimeModules = applicationConfiguration->getProperty< SmallSet< std::wstring > >(L"Runtime.Modules");

	// Launch migration through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	Ref< Environment > env = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathName());
#else
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathNameNoVolume());
#endif
	env->set(L"DEPLOY_PROJECT_NAME", m_targetName);
	env->set(L"DEPLOY_PROJECT_IDENTIFIER", m_target->getIdentifier());
	env->set(L"DEPLOY_PROJECT_VERSION", m_target->getVersion());
	env->set(L"DEPLOY_PROJECT_ICON", m_targetConfiguration->getIcon());
	env->set(L"DEPLOY_SYSTEM_ROOT", m_globalSettings->getProperty< std::wstring >(L"Runtime.SystemRoot", L"$(TRAKTOR_HOME)"));
	env->set(L"DEPLOY_TARGET_HOST", m_deployHost);
	env->set(L"DEPLOY_EXECUTABLE", executableFile);
	env->set(L"DEPLOY_MODULES", implode(runtimeModules.begin(), runtimeModules.end(), L" "));
	env->set(L"DEPLOY_OUTPUT_PATH", m_outputPath);
	env->set(L"DEPLOY_DEBUG", m_globalSettings->getProperty< bool >(L"Runtime.UseDebugBinaries", false) ? L"YES" : L"");
	env->set(L"DEPLOY_STATIC_LINK", m_globalSettings->getProperty< bool >(L"Runtime.StaticallyLinked", false) ? L"YES" : L"");

	env->set(L"DEPLOY_ANDROID_HOME", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"), 0));
	env->set(L"DEPLOY_ANDROID_NDK_ROOT", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"), 0));
	env->set(L"DEPLOY_ANDROID_TOOLCHAIN", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));
	env->set(L"DEPLOY_ANDROID_APILEVEL", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	// Flatten feature deploy variables.
	const auto& values = deploy->getValues();
	for (auto i = values.begin(); i != values.end(); ++i)
		env->set(i->first, implodePropertyValue(i->second));

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	env->insert(deployTool.getEnvironment());

	// Merge all feature environment variables.
	for (auto feature : features)
		env->insert(feature->getEnvironment());

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable() + L" migrate",
		m_outputPath,
		env,
		OS::EfRedirectStdIO | OS::EfMute
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << deployTool.getExecutable() << L"\"" << Endl;
		return false;
	}

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	std::list< std::wstring > errors;
	std::wstring str;

	for (;;)
	{
		Ref< IStream > pipe;
		IProcess::WaitPipeResult result = process->waitPipeStream(100, pipe);
		if (result == IProcess::Ready && pipe != nullptr)
		{
			if (pipe == process->getPipeStream(IProcess::SpStdOut))
			{
				PipeReader::Result result;
				while ((result = stdOutReader.readLine(str)) == PipeReader::RtOk)
				{
					std::wstring tmp = trim(str);
					if (!tmp.empty() && tmp[0] == L':')
					{
						std::vector< std::wstring > out;
						if (Split< std::wstring >::any(tmp, L":", out) == 2)
						{
							int32_t index = parseString< int32_t >(out[0]);
							int32_t count = parseString< int32_t >(out[1]);
							if (count > 0)
							{
								if (progressListener)
									progressListener->notifyTargetActionProgress(2 + (98 * index) / count, 100);
							}
						}
					}
					else
						log::info << str << Endl;
				}
			}
			else if (pipe == process->getPipeStream(IProcess::SpStdErr))
			{
				PipeReader::Result result;
				while ((result = stdErrReader.readLine(str)) == PipeReader::RtOk)
				{
					str = trim(str);
					if (!str.empty())
					{
						log::error << str << Endl;
						errors.push_back(str);
					}
				}
			}
		}
		else if (result == IProcess::Terminated)
			break;
	}

	if (!errors.empty())
	{
		log::error << L"Unsuccessful migrate, error(s):" << Endl;
		for (std::list< std::wstring >::const_iterator i = errors.begin(); i != errors.end(); ++i)
			log::error << L"\t" << *i << Endl;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process \"" << deployTool.getExecutable() << L" migrate\" failed with exit code " << exitCode << L"." << Endl;

	return exitCode == 0;
}

}
