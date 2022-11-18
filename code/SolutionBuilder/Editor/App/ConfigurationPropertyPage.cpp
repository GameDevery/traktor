/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Editor/App/ConfigurationPropertyPage.h"
#include "Ui/Application.h"
#include "Ui/Static.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace sb
	{
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ConfigurationPropertyPage", ConfigurationPropertyPage, ui::Container)

bool ConfigurationPropertyPage::create(ui::Widget* parent)
{
	const int32_t f = ui::dpi96(4);

	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"100%", f, f)
	))
		return false;

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(this, ui::Tab::WsLine | ui::WsDoubleBuffer);

	Ref< ui::TabPage > tabPageBuild = new ui::TabPage();
	tabPageBuild->create(tab, L"Build", new ui::TableLayout(L"*,100%", L"*,*,100%,100%,100%,100%,*,*,*", f, f));
	tab->addPage(tabPageBuild);

	Ref< ui::TabPage > tabPageDebug = new ui::TabPage();
	tabPageDebug->create(tab, L"Debug", new ui::TableLayout(L"*,100%", L"*", f, f));
	tab->addPage(tabPageDebug);

	Ref< ui::TabPage > tabPageConsumer = new ui::TabPage();
	tabPageConsumer->create(tab, L"Consumer", new ui::TableLayout(L"*,100%", L"*", f, f));
	tab->addPage(tabPageConsumer);

	tab->setActivePage(tabPageBuild);

	// Build
	Ref< ui::Static > staticType = new ui::Static();
	staticType->create(tabPageBuild, L"Type");

	m_dropType = new ui::DropDown();
	m_dropType->create(tabPageBuild);
	m_dropType->add(L"Static library");
	m_dropType->add(L"Shared library");
	m_dropType->add(L"Executable");
	m_dropType->add(L"Executable (console)");
	m_dropType->addEventHandler< ui::SelectionChangeEvent >(this, &ConfigurationPropertyPage::eventSelectType);

	Ref< ui::Static > staticProfile = new ui::Static();
	staticProfile->create(tabPageBuild, L"Profile");

	m_dropProfile = new ui::DropDown();
	m_dropProfile->create(tabPageBuild);
	m_dropProfile->add(L"Debug");
	m_dropProfile->add(L"Release");
	m_dropProfile->addEventHandler< ui::SelectionChangeEvent >(this, &ConfigurationPropertyPage::eventSelectProfile);

	Ref< ui::Static > staticIncludePaths = new ui::Static();
	staticIncludePaths->create(tabPageBuild, L"Include paths");
	staticIncludePaths->setVerticalAlign(ui::AnTop);

	m_listIncludePaths = new ui::EditList();
	m_listIncludePaths->create(tabPageBuild, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove | ui::EditList::WsSingle);
	m_listIncludePaths->addEventHandler< ui::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeIncludePath);

	Ref< ui::Static > staticDefinitions = new ui::Static();
	staticDefinitions->create(tabPageBuild, L"Definitions");

	m_listDefinitions = new ui::EditList();
	m_listDefinitions->create(tabPageBuild, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove | ui::EditList::WsSingle);
	m_listDefinitions->addEventHandler< ui::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeDefinitions);

	Ref< ui::Static > staticLibraryPaths = new ui::Static();
	staticLibraryPaths->create(tabPageBuild, L"Library paths");

	m_listLibraryPaths = new ui::EditList();
	m_listLibraryPaths->create(tabPageBuild, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove | ui::EditList::WsSingle);
	m_listLibraryPaths->addEventHandler< ui::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeLibraryPaths);

	Ref< ui::Static > staticLibraries = new ui::Static();
	staticLibraries->create(tabPageBuild, L"Libraries");

	m_listLibraries = new ui::EditList();
	m_listLibraries->create(tabPageBuild, ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove | ui::EditList::WsSingle);
	m_listLibraries->addEventHandler< ui::EditListEditEvent >(this, &ConfigurationPropertyPage::eventChangeLibraries);

	Ref< ui::Static > staticWarningLevel = new ui::Static();
	staticWarningLevel->create(tabPageBuild, L"Warning level");

	m_dropWarningLevel = new ui::DropDown();
	m_dropWarningLevel->create(tabPageBuild);
	m_dropWarningLevel->add(L"No warnings");
	m_dropWarningLevel->add(L"Critical warnings only");
	m_dropWarningLevel->add(L"Compiler default");
	m_dropWarningLevel->add(L"All warnings");
	m_dropWarningLevel->addEventHandler< ui::SelectionChangeEvent >(this, &ConfigurationPropertyPage::eventSelectWarningLevel);

	Ref< ui::Static > staticAdditionalCompilerOptions = new ui::Static();
	staticAdditionalCompilerOptions->create(tabPageBuild, L"Compiler options");

	m_editAdditionalCompilerOptions = new ui::Edit();
	m_editAdditionalCompilerOptions->create(tabPageBuild);
	m_editAdditionalCompilerOptions->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticAdditionalLinkerOptions = new ui::Static();
	staticAdditionalLinkerOptions->create(tabPageBuild, L"Linker options");

	m_editAdditionalLinkerOptions = new ui::Edit();
	m_editAdditionalLinkerOptions->create(tabPageBuild);
	m_editAdditionalLinkerOptions->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	// Debug
	Ref< ui::Static > staticDebugExecutable = new ui::Static();
	staticDebugExecutable->create(tabPageDebug, L"Executable");

	m_editDebugExecutable = new ui::Edit();
	m_editDebugExecutable->create(tabPageDebug);
	m_editDebugExecutable->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugArguments = new ui::Static();
	staticDebugArguments->create(tabPageDebug, L"Arguments");

	m_editDebugArguments = new ui::Edit();
	m_editDebugArguments->create(tabPageDebug);
	m_editDebugArguments->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugEnvironment = new ui::Static();
	staticDebugEnvironment->create(tabPageDebug, L"Environment");

	m_editDebugEnvironment = new ui::Edit();
	m_editDebugEnvironment->create(tabPageDebug);
	m_editDebugEnvironment->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	Ref< ui::Static > staticDebugWorkingDirectory = new ui::Static();
	staticDebugWorkingDirectory->create(tabPageDebug, L"Working directory");

	m_editDebugWorkingDirectory = new ui::Edit();
	m_editDebugWorkingDirectory->create(tabPageDebug);
	m_editDebugWorkingDirectory->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	// Consumer
	Ref< ui::Static > staticConsumerLibraryPath = new ui::Static();
	staticConsumerLibraryPath->create(tabPageConsumer, L"Library path");

	m_editConsumerLibraryPath = new ui::Edit();
	m_editConsumerLibraryPath->create(tabPageConsumer);
	m_editConsumerLibraryPath->addEventHandler< ui::FocusEvent >(this, &ConfigurationPropertyPage::eventFocusAdditionalOptions);

	fit(Container::Both);
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

	m_dropWarningLevel->select(int(m_configuration->getWarningLevel()));

	m_editAdditionalCompilerOptions->setText(m_configuration->getAdditionalCompilerOptions());
	m_editAdditionalLinkerOptions->setText(m_configuration->getAdditionalLinkerOptions());

	m_editDebugExecutable->setText(m_configuration->getDebugExecutable());
	m_editDebugArguments->setText(m_configuration->getDebugArguments());
	m_editDebugEnvironment->setText(m_configuration->getDebugEnvironment());
	m_editDebugWorkingDirectory->setText(m_configuration->getDebugWorkingDirectory());

	m_editConsumerLibraryPath->setText(m_configuration->getConsumerLibraryPath());
}

void ConfigurationPropertyPage::eventSelectType(ui::SelectionChangeEvent* event)
{
	int id = m_dropType->getSelected();
	m_configuration->setTargetFormat((Configuration::TargetFormat)id);
}

void ConfigurationPropertyPage::eventSelectProfile(ui::SelectionChangeEvent* event)
{
	int id = m_dropProfile->getSelected();
	m_configuration->setTargetProfile((Configuration::TargetProfile)id);
}

void ConfigurationPropertyPage::eventChangeIncludePath(ui::EditListEditEvent* event)
{
	std::vector< std::wstring > includePaths = m_configuration->getIncludePaths();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			includePaths[editId] = text;
		else
			includePaths.erase(includePaths.begin() + editId);
	}
	else
		includePaths.push_back(event->getText());
	m_configuration->setIncludePaths(includePaths);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeDefinitions(ui::EditListEditEvent* event)
{
	std::vector< std::wstring > definitions = m_configuration->getDefinitions();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			definitions[editId] = text;
		else
			definitions.erase(definitions.begin() + editId);
	}
	else
		definitions.push_back(event->getText());
	m_configuration->setDefinitions(definitions);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeLibraryPaths(ui::EditListEditEvent* event)
{
	std::vector< std::wstring > libraryPaths = m_configuration->getLibraryPaths();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			libraryPaths[editId] = text;
		else
			libraryPaths.erase(libraryPaths.begin() + editId);
	}
	else
		libraryPaths.push_back(event->getText());
	m_configuration->setLibraryPaths(libraryPaths);
	event->consume();
}

void ConfigurationPropertyPage::eventChangeLibraries(ui::EditListEditEvent* event)
{
	std::vector< std::wstring > libraries = m_configuration->getLibraries();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			libraries[editId] = text;
		else
			libraries.erase(libraries.begin() + editId);
	}
	else
		libraries.push_back(event->getText());
	m_configuration->setLibraries(libraries);
	event->consume();
}

void ConfigurationPropertyPage::eventSelectWarningLevel(ui::SelectionChangeEvent* event)
{
	int id = m_dropWarningLevel->getSelected();
	m_configuration->setWarningLevel((Configuration::WarningLevel)id);
}

void ConfigurationPropertyPage::eventFocusAdditionalOptions(ui::FocusEvent* event)
{
	m_configuration->setAdditionalCompilerOptions(m_editAdditionalCompilerOptions->getText());
	m_configuration->setAdditionalLinkerOptions(m_editAdditionalLinkerOptions->getText());

	m_configuration->setDebugExecutable(m_editDebugExecutable->getText());
	m_configuration->setDebugArguments(m_editDebugArguments->getText());
	m_configuration->setDebugEnvironment(m_editDebugEnvironment->getText());
	m_configuration->setDebugWorkingDirectory(m_editDebugWorkingDirectory->getText());

	m_configuration->setConsumerLibraryPath(m_editConsumerLibraryPath->getText());
}

	}
}
