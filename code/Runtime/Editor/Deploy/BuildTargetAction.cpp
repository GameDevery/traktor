#include "Runtime/Editor/Deploy/BuildTargetAction.h"
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
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
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace runtime
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
		std::vector< std::wstring > ss = PropertyStringArray::get(valueStringArray);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else if (const PropertyStringSet* valueStringSet = dynamic_type_cast< const PropertyStringSet* >(value))
	{
		std::set< std::wstring > ss = PropertyStringSet::get(valueStringSet);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else
		return L"";
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.BuildTargetAction", BuildTargetAction, ITargetAction)

BuildTargetAction::BuildTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const PropertyGroup* defaultPipelineSettings,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& outputPath,
	const PropertyGroup* tweakSettings,
	bool standAlone,
	bool force
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_defaultPipelineSettings(defaultPipelineSettings)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_outputPath(outputPath)
,	m_tweakSettings(tweakSettings)
,	m_standAlone(standAlone)
,	m_force(force)
{
}

bool BuildTargetAction::execute(IProgressListener* progressListener)
{
	Ref< PropertyGroup > deploy = new PropertyGroup();

	// Get platform description object from database.
	Ref< Platform > platform = m_database->getObjectReadOnly< Platform >(m_targetConfiguration->getPlatform());
	if (!platform)
	{
		log::error << L"Unable to get platform object" << Endl;
		return false;
	}

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 100);

	// Create target pipeline configuration.
	Ref< PropertyGroup > pipelineConfiguration = new PropertyGroup();
	if (m_defaultPipelineSettings)
		pipelineConfiguration = DeepClone(m_defaultPipelineSettings).create< PropertyGroup >();

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

	// Insert target's features into pipeline configuration.
	for (RefArray< const Feature >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		const Feature* feature = *i;
		T_ASSERT(feature);

		const Feature::Platform* fp = feature->getPlatform(m_targetConfiguration->getPlatform());
		if (fp)
		{
			if (fp->deploy)
				deploy = deploy->merge(fp->deploy, PropertyGroup::MmJoin);
		}
		else
			log::warning << L"Feature \"" << feature->getDescription() << L"\" doesn't support selected platform." << Endl;

		Ref< const PropertyGroup > pipelineProperties = feature->getPipelineProperties();
		if (!pipelineProperties)
			continue;

		pipelineConfiguration = pipelineConfiguration->merge(pipelineProperties, PropertyGroup::MmJoin);
	}

	// Merge pipeline cache configuration from global configuration.
	bool inheritCache = m_globalSettings->getProperty< bool >(L"Runtime.InheritCache", true);
	if (inheritCache)
	{
		bool fileCacheEnable = m_globalSettings->getProperty< bool >(L"Pipeline.FileCache", false);
		if (fileCacheEnable)
		{
			std::wstring fileCachePath = m_globalSettings->getProperty< std::wstring >(L"Pipeline.FileCache.Path", L"");
			fileCachePath = FileSystem::getInstance().getAbsolutePath(fileCachePath).getPathName();
			if (!fileCachePath.empty())
			{
				bool fileCacheRead = m_globalSettings->getProperty< bool >(L"Pipeline.FileCache.Read", false);
				bool fileCacheWrite = m_globalSettings->getProperty< bool >(L"Pipeline.FileCache.Write", false);

				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache", true);
				pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.FileCache.Path", fileCachePath);
				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", fileCacheRead);
				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", fileCacheWrite);
			}
		}

		bool memCachedEnable = m_globalSettings->getProperty< bool >(L"Pipeline.MemCached", false);
		if (memCachedEnable)
		{
			std::wstring memCachedHost = m_globalSettings->getProperty< std::wstring >(L"Pipeline.MemCached.Host", L"");
			int32_t memCachedPort = m_globalSettings->getProperty< int32_t >(L"Pipeline.MemCached.Port", 0);
			bool memCachedRead = m_globalSettings->getProperty< bool >(L"Pipeline.MemCached.Read", false);
			bool memCachedWrite = m_globalSettings->getProperty< bool >(L"Pipeline.MemCached.Write", false);

			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached", true);
			pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.MemCached.Host", memCachedHost);
			pipelineConfiguration->setProperty< PropertyInteger >(L"Pipeline.MemCached.Port", memCachedPort);
			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", memCachedRead);
			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", memCachedWrite);
		}
	}

	// Set instance cache path.
	std::wstring instanceCachePath = m_globalSettings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");
	instanceCachePath = FileSystem::getInstance().getAbsolutePath(instanceCachePath).getPathName();
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.InstanceCache.Path", instanceCachePath);

	// Merge threaded build configuration from global configuration.
	bool dependsThreads = m_globalSettings->getProperty< bool >(L"Pipeline.DependsThreads", true);
	pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.DependsThreads", dependsThreads);
	bool buildThreads = m_globalSettings->getProperty< bool >(L"Pipeline.BuildThreads", true);
	pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.BuildThreads", buildThreads);

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs = m_globalSettings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	db::ConnectionString outputDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=" + FileSystem::getInstance().getAbsolutePath(m_outputPath + L"/db").getPathName() + L";binary=true;fileStore=traktor.db.NoFileStore");

	if (sourceDatabaseCs.have(L"groupPath"))
	{
		Path groupPath = FileSystem::getInstance().getAbsolutePath(sourceDatabaseCs.get(L"groupPath"));
		sourceDatabaseCs.set(L"groupPath", groupPath.getPathName());
	}

	sourceDatabaseCs.set(L"fileStore", L"");

	pipelineConfiguration->setProperty< PropertyString >(L"Editor.SourceDatabase", sourceDatabaseCs.format());
	pipelineConfiguration->setProperty< PropertyString >(L"Editor.OutputDatabase", outputDatabaseCs.format());

	// Set data access cache path.
	Path dataAccessCachePath = m_globalSettings->getProperty< std::wstring >(L"Pipeline.DataAccessCache.Path");
	dataAccessCachePath = FileSystem::getInstance().getAbsolutePath(dataAccessCachePath);
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.DataAccessCache.Path", dataAccessCachePath.getPathName());

	// Set asset path.
	Path assetPath = m_globalSettings->getProperty< std::wstring >(L"Pipeline.AssetPath");
	assetPath = FileSystem::getInstance().getAbsolutePath(assetPath);
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.AssetPath", assetPath.getPathName());

	// Set model cache path.
	Path modelCachePath = m_globalSettings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
	modelCachePath = FileSystem::getInstance().getAbsolutePath(modelCachePath);
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.ModelCache.Path", modelCachePath.getPathName());

	// Set shader program cache path.
	Path programCachePath = m_globalSettings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCache.Path");
	programCachePath = FileSystem::getInstance().getAbsolutePath(programCachePath);
	pipelineConfiguration->setProperty< PropertyString >(L"ShaderPipeline.ProgramCache.Path", programCachePath.getPathName());

	// Insert mesh pipeline's material templates.
	const IPropertyValue* materialTemplates = m_globalSettings->getProperty(L"MeshPipeline.MaterialTemplates");
	if (materialTemplates)
		pipelineConfiguration->setProperty(L"MeshPipeline.MaterialTemplates", materialTemplates->clone());

	// Append tweaks.
	if (m_tweakSettings)
		pipelineConfiguration = pipelineConfiguration->merge(m_tweakSettings, PropertyGroup::MmJoin);

	// Ensure output directory exists.
	if (!FileSystem::getInstance().makeAllDirectories(m_outputPath))
	{
		log::error << L"Unable to create output path \"" << m_outputPath << L"\"" << Endl;
		return false;
	}

	// Write generated pipeline configuration in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
		m_outputPath + L"/Pipeline.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(pipelineConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write pipeline configuration" << Endl;
		return false;
	}

	// Launch pipeline through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	Ref< Environment > env = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathName());
#else
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathNameNoVolume());
#endif
	env->set(L"DEPLOY_SYSTEM_ROOT", m_globalSettings->getProperty< std::wstring >(L"Runtime.SystemRoot", L"$(TRAKTOR_HOME)"));
	env->set(L"DEPLOY_OUTPUT_PATH", m_outputPath);
	env->set(L"DEPLOY_DEBUG", m_globalSettings->getProperty< bool >(L"Runtime.UseDebugBinaries", false) ? L"YES" : L"");
	env->set(L"DEPLOY_STATIC_LINK", m_globalSettings->getProperty< bool >(L"Runtime.StaticallyLinked", false) ? L"YES" : L"");

	env->set(L"DEPLOY_ANDROID_HOME", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"), 0));
	env->set(L"DEPLOY_ANDROID_NDK_ROOT", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"), 0));
	env->set(L"DEPLOY_ANDROID_TOOLCHAIN", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));
	env->set(L"DEPLOY_ANDROID_APILEVEL", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	env->set(L"DEPLOY_EMSCRIPTEN", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.Emscripten", L"$(EMSDK)"), 0));

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

	StringOutputStream ss;
	ss << deployTool.getExecutable() << L" build";

	Guid root = m_targetConfiguration->getRoot();
	if (root.isValid() && !root.isNull())
		ss << L" " << root.format();

	Guid startup = m_targetConfiguration->getStartup();
	if (startup.isValid() && !startup.isNull())
		ss << L" " << startup.format();

	Guid defaultInput = m_targetConfiguration->getDefaultInput();
	if (defaultInput.isValid() && !defaultInput.isNull())
		ss << L" " << defaultInput.format();

	Guid onlineConfig = m_targetConfiguration->getOnlineConfig();
	if (onlineConfig.isValid() && !onlineConfig.isNull())
		ss << L" " << onlineConfig.format();

	for (auto feature : features)
	{
		for (const auto& dependency : feature->getDependencies())
			ss << L" " << dependency.format();
	}

	if (m_globalSettings->getProperty< bool >(L"Pipeline.Verbose", false))
		ss << L" -verbose";
	if (m_standAlone)
		ss << L" -standalone";
	if (m_force)
		ss << L" -force";

	Ref< IProcess > process = OS::getInstance().execute(
		ss.str(),
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
		PipeReader::Result result1 = stdOutReader.readLine(str, 10);
		if (result1 == PipeReader::RtOk)
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

		PipeReader::Result result2 = stdErrReader.readLine(str, 10);
		if (result2 == PipeReader::RtOk)
		{
			str = trim(str);
			if (!str.empty())
			{
				log::error << str << Endl;
				errors.push_back(str);
			}
		}

		if (result1 == PipeReader::RtEnd && result2 == PipeReader::RtEnd)
			break;
	}

	if (!errors.empty())
	{
		log::error << L"Unsuccessful build, error(s):" << Endl;
		for (std::list< std::wstring >::const_iterator i = errors.begin(); i != errors.end(); ++i)
			log::error << L"\t" << *i << Endl;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << Endl;

	return exitCode == 0;
}

	}
}
