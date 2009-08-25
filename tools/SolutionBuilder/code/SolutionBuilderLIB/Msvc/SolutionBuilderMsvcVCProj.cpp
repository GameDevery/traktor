#include <Core/Io/FileSystem.h>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Serialization/MemberRef.h>
#include <Core/Misc/String.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcConfiguration.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcVCProj", SolutionBuilderMsvcVCProj, SolutionBuilderMsvcProject)

std::wstring SolutionBuilderMsvcVCProj::getPlatform() const
{
	return m_platform;
}

bool SolutionBuilderMsvcVCProj::getInformation(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	std::wstring& outProjectPath,
	std::wstring& outProjectFileName,
	std::wstring& outProjectGuid
) const
{
	outProjectPath = solution->getRootPath() + L"/" + project->getName();
	outProjectFileName = outProjectPath + L"/" + project->getName() + L".vcproj";
	outProjectGuid = context.generateGUID(outProjectFileName);
	return true;
}

bool SolutionBuilderMsvcVCProj::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project
) const
{
	std::wstring projectPath, projectFileName, projectGuid;
	if (!getInformation(context, solution, project, projectPath, projectFileName, projectGuid))
		return false;

	if (!FileSystem::getInstance().makeDirectory(projectPath))
		return false;

	traktor::log::info << L"Generating vs project \"" << projectFileName << L"\"" << Endl;

	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	std::vector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, gc_new< AnsiEncoding >());

	os << L"<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
	os << L"<VisualStudioProject" << Endl;
	os << IncreaseIndent;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << L">" << Endl;

	os << L"<Platforms>" << Endl;
	os << IncreaseIndent;
	os << L"<Platform" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"" << m_platform << L"\"" << Endl;
	os << DecreaseIndent;
	os << L"/>" << Endl;
	os << DecreaseIndent;
	os << L"</Platforms>" << Endl;

	os << L"<ToolFiles/>" << Endl;

	os << L"<Configurations>" << Endl;
	os << IncreaseIndent;

	RefList< Configuration >& configurations = project->getConfigurations();
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		context.set(L"CONFIGURATION_NAME", (*i)->getName());

		const SolutionBuilderMsvcConfiguration* configuration = m_configurations[int((*i)->getTargetFormat())];
		if (!configuration->generate(context, solution, project, *i, m_platform, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Configurations>" << Endl;

	os << L"<References/>" << Endl;

	os << L"<Files>" << Endl;
	os << IncreaseIndent;

	RefList< ProjectItem >& items = project->getItems();
	for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		if (!addItem(context, solution, project, *i, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Files>" << Endl;

	os << DecreaseIndent;
	os << L"</VisualStudioProject>" << Endl;

	os.close();

	if (!buffer.empty())
	{
		Ref< Stream > file = FileSystem::getInstance().open(
			projectFileName,
			traktor::File::FmWrite
		);
		if (!file)
			return false;
		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	return true;
}

bool SolutionBuilderMsvcVCProj::serialize(traktor::Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	s >> Member< std::wstring >(L"platform", m_platform);
	s >> MemberStaticArray< Ref< SolutionBuilderMsvcConfiguration >, 4, MemberRef< SolutionBuilderMsvcConfiguration > >(L"configurations", m_configurations);
	s >> MemberStlMap<
		std::wstring,
		Ref< SolutionBuilderMsvcTool >,
		MemberStlPair<
			std::wstring,
			Ref< SolutionBuilderMsvcTool >,
			Member< std::wstring >,
			MemberRef< SolutionBuilderMsvcTool >
		>
	>(L"customTools", m_customTools);
	return true;
}

bool SolutionBuilderMsvcVCProj::addItem(GeneratorContext& context, Solution* solution, Project* project, ProjectItem* item, OutputStream& os) const
{
	Ref< Filter > filter = dynamic_type_cast< Filter* >(item);
	if (filter)
	{
		os << L"<Filter" << Endl;
		os << IncreaseIndent;
		os << L"Name=\"" << filter->getName() << L"\"" << Endl;
		os << L">" << Endl;

		RefList< ProjectItem >& items = item->getItems();
		for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
			addItem(context, solution, project, *i, os);

		os << DecreaseIndent;
		os << L"</Filter>" << Endl;
	}

	Ref< ::File > file = dynamic_type_cast< ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		Path rootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			Path systemPath = FileSystem::getInstance().getAbsolutePath(*i);

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				systemPath,
				rootPath,
				relativePath
			);

			std::map< std::wstring, traktor::Ref< SolutionBuilderMsvcTool > >::const_iterator j = m_customTools.find(toLower(relativePath.getExtension()));
			if (j == m_customTools.end())
			{
				// Unable to find any custom build rule; assume it's directly "compilable" by default.
				os << L"<File" << Endl;
				os << IncreaseIndent;
				os << L"RelativePath=\"..\\" << replaceAll< std::wstring >(relativePath.getPathName(), '/', '\\') << L"\"" << Endl;
				os << DecreaseIndent;
				os << L"/>" << Endl;
			}
			else
			{
				// Create custom build rule for this type of file.
				os << L"<File" << Endl;
				os << IncreaseIndent;
				os << L"RelativePath=\"..\\" << replaceAll< std::wstring >(relativePath.getPathName(), '/', '\\') << L"\"" << Endl;
				os << DecreaseIndent;
				os << L">" << Endl;
				os << IncreaseIndent;

				// Add custom configurations to this file.
				RefList< Configuration >& configurations = project->getConfigurations();
				for (RefList< Configuration >::iterator k = configurations.begin(); k != configurations.end(); ++k)
				{
					os << L"<FileConfiguration Name=\"" << (*k)->getName() << L"|" << m_platform << L"\">" << Endl;
					os << IncreaseIndent;

					context.set(L"CONFIGURATION_NAME", (*k)->getName());

					if (!j->second->generate(context, solution, project, *k, os))
						return false;

					os << DecreaseIndent;
					os << L"</FileConfiguration>" << Endl;
				}

				os << DecreaseIndent;
				os << L"</File>" << Endl;
			}
		}
	}

	return true;
}