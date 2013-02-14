#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/Tool/DeployTargetAction.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Net/SocketAddressIPv4.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.DeployTargetAction", DeployTargetAction, ITargetAction)

DeployTargetAction::DeployTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const std::wstring& targetName,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& deployHost,
	const std::wstring& databaseName,
	const Guid& targetManagerId,
	const std::wstring& outputPath,
	const PropertyGroup* tweakSettings
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_targetName(targetName)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_deployHost(deployHost)
,	m_databaseName(databaseName)
,	m_targetManagerId(targetManagerId)
,	m_outputPath(outputPath)
,	m_tweakSettings(tweakSettings)
{
}

bool DeployTargetAction::execute(IProgressListener* progressListener)
{
	// Get platform description object from database.
	Ref< Platform > platform = m_database->getObjectReadOnly< Platform >(m_targetConfiguration->getPlatform());
	if (!platform)
	{
		log::error << L"Unable to get platform object" << Endl;
		return false;
	}

	if (progressListener)
		progressListener->notifyTargetActionProgress(1, 2);

	// Create target application configuration.
	Ref< PropertyGroup > applicationConfiguration = new PropertyGroup();

	// Insert features into runtime configuration.
	const std::list< Guid >& features = m_targetConfiguration->getFeatures();
	for (std::list< Guid >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(*i);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << i->format() << L"\"; feature skipped." << Endl;
			continue;
		}

		Ref< const PropertyGroup > runtimeProperties = feature->getRuntimeProperties();
		if (!runtimeProperties)
			continue;

		applicationConfiguration = applicationConfiguration->mergeJoin(runtimeProperties);
	}

	int32_t remoteDatabasePort = m_globalSettings->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort);
	int32_t targetManagerPort = m_globalSettings->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort);

	// Determine our interface address; we let applications know where to find data.
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
	{
		traktor::log::error << L"Unable to get interfaces" << Endl;
		return false;
	}

	std::wstring host = itf.addr->getHostName();

	// Modify configuration to connect to embedded database server.
	db::ConnectionString remoteCs;
	remoteCs.set(L"provider", L"traktor.db.RemoteDatabase");
	remoteCs.set(L"host", host + L":" + toString(remoteDatabasePort));
	remoteCs.set(L"database", m_databaseName);
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Database", remoteCs.format());

	// Modify configuration to connect to embedded target manager.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Host", host);
	applicationConfiguration->setProperty< PropertyInteger >(L"Amalgam.TargetManager/Port", targetManagerPort);
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Id", m_targetManagerId.format());

	// Append target guid;s to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Root", m_targetConfiguration->getRoot().format());
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Startup", m_targetConfiguration->getStartup().format());
	applicationConfiguration->setProperty< PropertyString >(L"Input.Default", m_targetConfiguration->getDefaultInput().format());
	applicationConfiguration->setProperty< PropertyString >(L"Online.Config", m_targetConfiguration->getOnlineConfig().format());

	// Append application title.
	applicationConfiguration->setProperty< PropertyString >(L"Render.Title", m_targetName);

	// Append tweaks.
	if (m_tweakSettings)
		applicationConfiguration = applicationConfiguration->mergeReplace(m_tweakSettings);

	// Write generated application configuration in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
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
	std::set< std::wstring > modules = applicationConfiguration->getProperty< PropertyStringSet >(L"Amalgam.Modules");
	std::wstring mm = implode(modules.begin(), modules.end(), L" ");

	// Launch deploy tool to ensure platform is ready for launch.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathNameNoVolume();
#endif
	envmap[L"DEPLOY_PROJECT_NAME"] = m_targetName;
	envmap[L"DEPLOY_PROJECT_IDENTIFIER"] = m_target->getIdentifier();
	envmap[L"DEPLOY_PROJECT_ICON"] = m_targetConfiguration->getIcon();
	envmap[L"DEPLOY_TARGET_HOST"] = m_deployHost;
	envmap[L"DEPLOY_EXECUTABLE"] = m_targetConfiguration->getExecutable();
	envmap[L"DEPLOY_MODULES"] = mm;
	envmap[L"DEPLOY_OUTPUT_PATH"] = m_outputPath;

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	envmap.insert(deployTool.getEnvironment().begin(), deployTool.getEnvironment().end());

	// Merge settings environment variables.
	Ref< PropertyGroup > settingsEnvironment = m_globalSettings->getProperty< PropertyGroup >(L"Amalgam.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				envmap.insert(std::make_pair(
					i->first,
					PropertyString::get(value)
				));
			}
		}
	}

	// Launch deploy process.
	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable(),
		L"deploy",
		m_outputPath,
		&envmap,
#if defined(_DEBUG)
		false, false, false
#else
		true, true, false
#endif
	);
	if (!process || !process->wait())
	{
		log::error << L"Failed to launch process \"" << deployTool.getExecutable() << L"\"" << Endl;
		return false;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << Endl;

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 2);

	return exitCode == 0;
}

	}
}
