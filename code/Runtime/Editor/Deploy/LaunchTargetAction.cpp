/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/System/ResolveEnv.h"
#include "Database/Database.h"
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/Deploy/LaunchTargetAction.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"

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
		const auto ss = PropertyStringArray::get(valueStringArray);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else if (const PropertyStringSet* valueStringSet = dynamic_type_cast< const PropertyStringSet* >(value))
	{
		const auto ss = PropertyStringSet::get(valueStringSet);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else
		return L"";
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.LaunchTargetAction", LaunchTargetAction, ITargetAction)

LaunchTargetAction::LaunchTargetAction(
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

bool LaunchTargetAction::execute(IProgressListener* progressListener)
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
		progressListener->notifyTargetActionProgress(1, 2);

	// Get features; sorted by priority.
	const std::list< Guid >& featureIds = m_targetConfiguration->getFeatures();

	RefArray< const Feature > features;
	for (std::list< Guid >::const_iterator i = featureIds.begin(); i != featureIds.end(); ++i)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(*i);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << i->format() << L"\"; feature skipped." << Endl;
			continue;
		}
		features.push_back(feature);
	}

	features.sort(FeaturePriorityPred());

	// Get executable file from features.
	for (RefArray< const Feature >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		const Feature* feature = *i;
		T_ASSERT(feature);

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

	// Launch application through deploy tool.
	const Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();

	Path outputRelativePath;
	FileSystem::getInstance().getRelativePath(m_outputPath, projectRoot, outputRelativePath);

	Ref< Environment > env = OS::getInstance().getEnvironment();
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathNameOS());
	env->set(L"DEPLOY_PROJECT_NAME", m_targetName);
	env->set(L"DEPLOY_PROJECT_IDENTIFIER", m_target->getIdentifier());
	env->set(L"DEPLOY_PROJECT_VERSION", m_target->getVersion());
	env->set(L"DEPLOY_PROJECT_ICON", m_targetConfiguration->getIcon());
	env->set(L"DEPLOY_SYSTEM_ROOT", m_globalSettings->getProperty< std::wstring >(L"Runtime.SystemRoot", L"$(TRAKTOR_HOME)"));
	env->set(L"DEPLOY_TARGET_HOST", m_deployHost);
	env->set(L"DEPLOY_EXECUTABLE", executableFile);
	env->set(L"DEPLOY_OUTPUT_PATH", m_outputPath);
	env->set(L"DEPLOY_OUTPUT_URL", L"http://localhost:44246/" + outputRelativePath.getPathName());
	env->set(L"DEPLOY_DEBUG", m_globalSettings->getProperty< bool >(L"Runtime.UseDebugBinaries", false) ? L"YES" : L"");
	env->set(L"DEPLOY_STATIC_LINK", m_globalSettings->getProperty< bool >(L"Runtime.StaticallyLinked", false) ? L"YES" : L"");

	env->set(L"DEPLOY_ANDROID_HOME", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"), 0));
	env->set(L"DEPLOY_ANDROID_NDK_ROOT", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"), 0));
	env->set(L"DEPLOY_ANDROID_TOOLCHAIN", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));
	env->set(L"DEPLOY_ANDROID_APILEVEL", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	// Flatten feature deploy variables.
	for (auto it : deploy->getValues())
		env->set(it.first, implodePropertyValue(it.second));

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	env->insert(deployTool.getEnvironment());

	// Merge all feature environment variables.
	for (auto feature : features)
		env->insert(feature->getEnvironment());

	// Merge settings environment variables.
	Ref< PropertyGroup > settingsEnvironment = m_globalSettings->getProperty< PropertyGroup >(L"Runtime.Environment");
	if (settingsEnvironment)
	{
		const auto& values = settingsEnvironment->getValues();
		for (auto i = values.begin(); i != values.end(); ++i)
		{
			const PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				env->set(
					i->first,
					PropertyString::get(value)
				);
			}
		}
	}

	// Launch deploy process.
	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable() + L" launch",
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
					const std::wstring tmp = trim(str);
					if (!tmp.empty() && tmp[0] == L':')
					{
						std::vector< std::wstring > out;
						if (Split< std::wstring >::any(tmp, L":", out) == 2)
						{
							const int32_t index = parseString< int32_t >(out[0]);
							const int32_t count = parseString< int32_t >(out[1]);
							if (count > 0)
							{
								if (progressListener)
									progressListener->notifyTargetActionProgress(2 + (98 * index) / count, 100);
							}
						}
					}
					else if (progressListener)
						progressListener->notifyLog(log::info.getLevel(), str);
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
						if (progressListener)
							progressListener->notifyLog(log::error.getLevel(), str);
						else
							log::error << str << Endl;
						errors.push_back(str);
					}
				}
			}
		}
		else if (result == IProcess::Terminated)
			break;
	}

	//if (!errors.empty())
	//{
	//	log::error << L"Unsuccessful build, error(s):" << Endl;
	//	for (std::list< std::wstring >::const_iterator i = errors.begin(); i != errors.end(); ++i)
	//		log::error << L"\t" << *i << Endl;
	//}

	const int32_t exitCode = process->exitCode();
	//if (exitCode != 0)
	//	log::error << L"Process \"" << deployTool.getExecutable() << L" launch\" failed with exit code " << exitCode << L"." << Endl;

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 2);

	return true;
}

}
