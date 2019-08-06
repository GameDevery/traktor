#include <set>
#include "Core/Log/Log.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Dependencies/SolutionBuilderDependencies.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

void collectDependencies(const Project* project, std::set< std::wstring >& outDependencies)
{
	outDependencies.insert(project->getName());
	for (auto dependency : project->getDependencies())
	{
		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			collectDependencies(projectDependency->getProject(), outDependencies);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
			collectDependencies(externalDependency->getProject(), outDependencies);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderDependencies", SolutionBuilderDependencies, SolutionBuilder)

bool SolutionBuilderDependencies::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption(L'p', L"project"))
		m_projectName = cmdLine.getOption(L'p', L"project").getString();

	return true;
}

bool SolutionBuilderDependencies::generate(Solution* solution)
{
	for (auto project : solution->getProjects())
	{
		if (m_projectName.empty() || project->getName() == m_projectName)
		{
			std::set< std::wstring > dependencies;
			collectDependencies(project, dependencies);
			for (const auto& dependency : dependencies)
				log::info << dependency << Endl;
			return true;
		}
	}
	return false;
}

void SolutionBuilderDependencies::showOptions() const
{
	log::info << L"\t-p,-project=Dependencies of project" << Endl;
}

	}
}
