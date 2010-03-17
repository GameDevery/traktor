#include <Core/Log/Log.h>
#include <Ui/MethodHandler.h>
#include <Ui/Static.h>
#include <Ui/TableLayout.h>
#include <Ui/Events/CommandEvent.h>
#include <Ui/Events/EditEvent.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderUI/ConfigurationPropertyPage.h"
#include "SolutionBuilderUI/EditList.h"

using namespace traktor;

namespace
{

	struct EmptyString
	{
		bool operator () (const std::wstring& s) const
		{
			return s.empty();
		}
	};

}

bool ConfigurationPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsClientBorder,
		new ui::TableLayout(L"*,100%", L"*,*,100%,100%,100%,100%,*,*", 4, 4)
	))
		return false;

	Ref< ui::Static > staticType = new ui::Static();
	staticType->create(this, L"Type");

	m_dropType = new ui::DropDown();
	m_dropType->create(this);
	m_dropType->add(L"Static library");
	m_dropType->add(L"Shared library");
	m_dropType->add(L"Executable");
	m_dropType->add(L"Executable (console)");
	m_dropType->addSelectEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventSelectType));

	Ref< ui::Static > staticProfile = new ui::Static();
	staticProfile->create(this, L"Profile");

	m_dropProfile = new ui::DropDown();
	m_dropProfile->create(this);
	m_dropProfile->add(L"Debug");
	m_dropProfile->add(L"Release");
	m_dropProfile->addSelectEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventSelectProfile));

	Ref< ui::Static > staticIncludePaths = new ui::Static();
	staticIncludePaths->create(this, L"Include paths");

	m_listIncludePaths = new EditList();
	m_listIncludePaths->create(this);
	m_listIncludePaths->addEditEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventChangeIncludePath));

	Ref< ui::Static > staticDefinitions = new ui::Static();
	staticDefinitions->create(this, L"Definitions");

	m_listDefinitions = new EditList();
	m_listDefinitions->create(this);
	m_listDefinitions->addEditEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventChangeDefinitions));

	Ref< ui::Static > staticLibraryPaths = new ui::Static();
	staticLibraryPaths->create(this, L"Library paths");

	m_listLibraryPaths = new EditList();
	m_listLibraryPaths->create(this);
	m_listLibraryPaths->addEditEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventChangeLibraryPaths));

	Ref< ui::Static > staticLibraries = new ui::Static();
	staticLibraries->create(this, L"Libraries");

	m_listLibraries = new EditList();
	m_listLibraries->create(this);
	m_listLibraries->addEditEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventChangeLibraries));

	Ref< ui::Static > staticAdditionalCompilerOptions = new ui::Static();
	staticAdditionalCompilerOptions->create(this, L"Compiler options");

	m_editAdditionalCompilerOptions = new ui::Edit();
	m_editAdditionalCompilerOptions->create(this);
	m_editAdditionalCompilerOptions->addFocusEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions));

	Ref< ui::Static > staticAdditionalLinkerOptions = new ui::Static();
	staticAdditionalLinkerOptions->create(this, L"Linker options");

	m_editAdditionalLinkerOptions = new ui::Edit();
	m_editAdditionalLinkerOptions->create(this);
	m_editAdditionalLinkerOptions->addFocusEventHandler(ui::createMethodHandler(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions));

	fit();

	return true;
}

void ConfigurationPropertyPage::set(Configuration* configuration)
{
	m_configuration = configuration;

	m_dropType->select(int(m_configuration->getTargetFormat()));
	m_dropProfile->select(int(m_configuration->getTargetProfile()));

	std::vector< std::wstring > includePaths = m_configuration->getIncludePaths();
	std::remove_if(includePaths.begin(), includePaths.end(), EmptyString());
	m_configuration->setIncludePaths(includePaths);

	std::vector< std::wstring > definitions = m_configuration->getDefinitions();
	std::remove_if(definitions.begin(), definitions.end(), EmptyString());
	m_configuration->setDefinitions(definitions);

	std::vector< std::wstring > libraryPaths = m_configuration->getLibraryPaths();
	std::remove_if(libraryPaths.begin(), libraryPaths.end(), EmptyString());
	m_configuration->setLibraryPaths(libraryPaths);

	std::vector< std::wstring > libraries = m_configuration->getLibraries();
	std::remove_if(libraries.begin(), libraries.end(), EmptyString());
	m_configuration->setLibraries(libraries);

	m_listIncludePaths->removeAll();
	for (std::vector< std::wstring >::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
		m_listIncludePaths->add(*i);

	m_listDefinitions->removeAll();
	for (std::vector< std::wstring >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
		m_listDefinitions->add(*i);

	m_listLibraryPaths->removeAll();
	for (std::vector< std::wstring >::const_iterator i = libraryPaths.begin(); i != libraryPaths.end(); ++i)
		m_listLibraryPaths->add(*i);

	m_listLibraries->removeAll();
	for (std::vector< std::wstring >::const_iterator i = libraries.begin(); i != libraries.end(); ++i)
		m_listLibraries->add(*i);

	m_editAdditionalCompilerOptions->setText(m_configuration->getAdditionalCompilerOptions());
	m_editAdditionalLinkerOptions->setText(m_configuration->getAdditionalLinkerOptions());
}

void ConfigurationPropertyPage::eventSelectType(ui::Event* event)
{
	int id = checked_type_cast< ui::CommandEvent* >(event)->getCommand().getId();
	m_configuration->setTargetFormat((Configuration::TargetFormat)id);
}

void ConfigurationPropertyPage::eventSelectProfile(ui::Event* event)
{
	int id = checked_type_cast< ui::CommandEvent* >(event)->getCommand().getId();
	m_configuration->setTargetProfile((Configuration::TargetProfile)id);
}

void ConfigurationPropertyPage::eventChangeIncludePath(ui::Event* event)
{
	std::vector< std::wstring > includePaths = m_configuration->getIncludePaths();
	int editId = static_cast< ui::EditEvent* >(event)->getParam();
	if (editId >= 0)
	{
		std::wstring text = static_cast< ui::EditEvent* >(event)->getText();
		if (!text.empty())
			includePaths[editId] = text;
		else
			includePaths.erase(includePaths.begin() + editId);
	}
	else
		includePaths.push_back(static_cast< ui::EditEvent* >(event)->getText());
	m_configuration->setIncludePaths(includePaths);
}

void ConfigurationPropertyPage::eventChangeDefinitions(ui::Event* event)
{
	std::vector< std::wstring > definitions = m_configuration->getDefinitions();
	int editId = static_cast< ui::EditEvent* >(event)->getParam();
	if (editId >= 0)
	{
		std::wstring text = static_cast< ui::EditEvent* >(event)->getText();
		if (!text.empty())
			definitions[editId] = text;
		else
			definitions.erase(definitions.begin() + editId);
	}
	else
		definitions.push_back(static_cast< ui::EditEvent* >(event)->getText());
	m_configuration->setDefinitions(definitions);
}

void ConfigurationPropertyPage::eventChangeLibraryPaths(ui::Event* event)
{
	std::vector< std::wstring > libraryPaths = m_configuration->getLibraryPaths();
	int editId = static_cast< ui::EditEvent* >(event)->getParam();
	if (editId >= 0)
	{
		std::wstring text = static_cast< ui::EditEvent* >(event)->getText();
		if (!text.empty())
			libraryPaths[editId] = text;
		else
			libraryPaths.erase(libraryPaths.begin() + editId);
	}
	else
		libraryPaths.push_back(static_cast< ui::EditEvent* >(event)->getText());
	m_configuration->setLibraryPaths(libraryPaths);
}

void ConfigurationPropertyPage::eventChangeLibraries(ui::Event* event)
{
	std::vector< std::wstring > libraries = m_configuration->getLibraries();
	int editId = static_cast< ui::EditEvent* >(event)->getParam();
	if (editId >= 0)
	{
		std::wstring text = static_cast< ui::EditEvent* >(event)->getText();
		if (!text.empty())
			libraries[editId] = text;
		else
			libraries.erase(libraries.begin() + editId);
	}
	else
		libraries.push_back(static_cast< ui::EditEvent* >(event)->getText());
	m_configuration->setLibraries(libraries);
}

void ConfigurationPropertyPage::eventFocusAdditionalOptions(ui::Event* event)
{
	m_configuration->setAdditionalCompilerOptions(m_editAdditionalCompilerOptions->getText());
	m_configuration->setAdditionalLinkerOptions(m_editAdditionalLinkerOptions->getText());
}
