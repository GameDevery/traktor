#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Button.h>
#include <Ui/ListViewItem.h>
#include <Ui/FileDialog.h>
#include <Ui/MessageBox.h>
#include <Ui/Custom/InputDialog.h>
#include <Ui/MethodHandler.h>
#include <Ui/Events/FocusEvent.h>
#include <Ui/Events/MouseEvent.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/SolutionLoader.h"
#include "ProjectPropertyPage.h"
#include "ImportProjectDialog.h"
using namespace traktor;

namespace
{

	struct DependencyPredicate
	{
		bool operator () (const Dependency* dep1, const Dependency* dep2) const
		{
			return dep1->getName().compare(dep2->getName()) < 0;
		}
	};

}

bool ProjectPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsClientBorder,
		gc_new< ui::TableLayout >(L"100%", L"*,100%", 4, 4)
	))
		return false;

	m_checkEnable = gc_new< ui::CheckBox >();
	m_checkEnable->create(this, L"Include project in build");
	m_checkEnable->addClickEventHandler(ui::createMethodHandler(this, &ProjectPropertyPage::eventEnableClick));

	Ref< ui::Container > container = gc_new< ui::Container >();
	container->create(this, ui::WsNone, gc_new< ui::TableLayout >(L"*,100%", L"*,100%,*", 0, 4));

	Ref< ui::Static > staticSourcePath = gc_new< ui::Static >();
	staticSourcePath->create(container, L"Source path");

	m_editSourcePath = gc_new< ui::Edit >();
	m_editSourcePath->create(container);
	m_editSourcePath->addFocusEventHandler(ui::createMethodHandler(this, &ProjectPropertyPage::eventFocusSource));

	Ref< ui::Static > staticDependencies = gc_new< ui::Static >();
	staticDependencies->create(container, L"Dependencies");

	m_listDependencies = gc_new< ui::ListView >();
	m_listDependencies->create(container, ui::WsClientBorder | ui::ListView::WsReport);
	m_listDependencies->addColumn(L"Dependency", 130);
	m_listDependencies->addColumn(L"Location", 180);
	m_listDependencies->addColumn(L"Link", 50);
	m_listDependencies->addDoubleClickEventHandler(ui::createMethodHandler(this, &ProjectPropertyPage::eventDependencyDoubleClick));

	Ref< ui::Static > staticAvailable = gc_new< ui::Static >();
	staticAvailable->create(container, L"Available");

	Ref< ui::Container > containerAvailable = gc_new< ui::Container >();
	containerAvailable->create(container, ui::WsNone, gc_new< ui::TableLayout >(L"100%,*,*,*", L"*", 0, 4));

	m_dropAvailable = gc_new< ui::DropDown >();
	m_dropAvailable->create(containerAvailable);

	Ref< ui::Button > buttonAdd = gc_new< ui::Button >();
	buttonAdd->create(containerAvailable, L"Add");
	buttonAdd->addClickEventHandler(ui::createMethodHandler(
		this,
		&ProjectPropertyPage::eventClickAdd
	));

	Ref< ui::Button > buttonRemove = gc_new< ui::Button >();
	buttonRemove->create(containerAvailable, L"Remove");
	buttonRemove->addClickEventHandler(ui::createMethodHandler(
		this,
		&ProjectPropertyPage::eventClickRemove
	));

	Ref< ui::Button > buttonAddExternal = gc_new< ui::Button >();
	buttonAddExternal->create(containerAvailable, L"External...");
	buttonAddExternal->addClickEventHandler(ui::createMethodHandler(
		this,
		&ProjectPropertyPage::eventClickAddExternal
	));

	return true;
}

void ProjectPropertyPage::set(Solution* solution, Project* project)
{
	m_solution = solution;
	m_project = project;

	m_checkEnable->setChecked(project->getEnable());
	m_editSourcePath->setText(project->getSourcePath());

	updateDependencyList();
}

void ProjectPropertyPage::updateDependencyList()
{
	RefList< Dependency > dependencies = m_project->getDependencies();
	Ref< ui::ListViewItems > dependencyItems = gc_new< ui::ListViewItems >();

	// Sort all dependencies.
	dependencies.sort(DependencyPredicate());

	// Add all local dependencies first.
	for (RefList< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ExternalDependency >(*i))
			continue;

		Ref< ui::ListViewItem > dependencyItem = gc_new< ui::ListViewItem >();
		dependencyItem->setText(0, (*i)->getName());
		dependencyItem->setText(1, (*i)->getLocation());
		dependencyItem->setText(2, (*i)->shouldLinkWithProduct() ? L"Yes" : L"No");
		dependencyItem->setData(L"DEPENDENCY", *i);
		dependencyItems->add(dependencyItem);
	}

	// Add external dependencies last.
	for (RefList< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ProjectDependency >(*i))
			continue;

		Ref< ui::ListViewItem > dependencyItem = gc_new< ui::ListViewItem >();
		dependencyItem->setText(0, (*i)->getName());
		dependencyItem->setText(1, (*i)->getLocation());
		dependencyItem->setData(L"DEPENDENCY", *i);
		dependencyItems->add(dependencyItem);
	}

	m_listDependencies->setItems(dependencyItems);

	// Get available projects, remove all local projects which are already in dependency list.
	RefList< Project > projects = m_solution->getProjects();
	for (RefList< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (!is_a< ProjectDependency >(*i))
			continue;

		RefList< Project >::iterator j = std::find(projects.begin(), projects.end(), static_cast< ProjectDependency* >(*i)->getProject());
		if (j != projects.end())
			projects.erase(j);
	}

	// Update drop down with available projects.
	m_dropAvailable->removeAll();
	for(RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if (*i != m_project)
			m_dropAvailable->add((*i)->getName());
	}
}

void ProjectPropertyPage::eventEnableClick(ui::Event* event)
{
	m_project->setEnable(m_checkEnable->isChecked());
}

void ProjectPropertyPage::eventFocusSource(ui::Event* event)
{
	if (checked_type_cast< ui::FocusEvent* >(event)->lostFocus())
		m_project->setSourcePath(m_editSourcePath->getText());
}

void ProjectPropertyPage::eventDependencyDoubleClick(ui::Event* event)
{
	ui::Point mousePosition = checked_type_cast< const ui::MouseEvent* >(event)->getPosition();

	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > dependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	if (!dependency)
		return;

	// Check if user double clicked on "link" column.
	int32_t left = m_listDependencies->getColumnWidth(0) + m_listDependencies->getColumnWidth(1);
	if (mousePosition.x < left)
	{
		Ref< ExternalDependency > selectedDependency = dynamic_type_cast< ExternalDependency* >(dependency);
		if (!selectedDependency)
			return;

		ui::custom::InputDialog::Field inputFields[] =
		{
			{ L"Location", selectedDependency->getSolutionFileName(), 0 }
		};

		ui::custom::InputDialog inputDialog;
		inputDialog.create(
			this,
			L"External dependency",
			L"Change location",
			inputFields,
			sizeof_array(inputFields)
		);
		if (inputDialog.showModal() == ui::DrOk)
		{
			selectedDependency->setSolutionFileName(inputFields[0].value);
			updateDependencyList();
		}
		inputDialog.destroy();
	}
	else
	{
		dependency->setLinkWithProduct(!dependency->shouldLinkWithProduct());
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickAdd(ui::Event* event)
{
	std::wstring dependencyName = m_dropAvailable->getSelectedItem();
	if (!dependencyName.empty())
	{
		const RefList< Project >& projects = m_solution->getProjects();
		for(RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			if ((*i)->getName() == dependencyName)
			{
				Ref< ProjectDependency > dependency = gc_new< ProjectDependency >(*i);
				m_project->addDependency(dependency);
				break;
			}
		}
		m_dropAvailable->select(-1);
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickRemove(ui::Event* event)
{
	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > selectedDependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	T_ASSERT (selectedDependency);

	RefList< Dependency > dependencies = m_project->getDependencies();
	RefList< Dependency >::iterator i = std::find(dependencies.begin(), dependencies.end(), selectedDependency);
	T_ASSERT (i != dependencies.end());

	dependencies.erase(i);
	m_project->setDependencies(dependencies);
	
	updateDependencyList();
}

void ProjectPropertyPage::eventClickAddExternal(ui::Event* event)
{
	ui::FileDialog fileDialog;
	fileDialog.create(this, L"Select solution", L"SolutionBuilder solutions;*.xms");

	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		Ref< Solution > solution = SolutionLoader().load(filePath.getPathName());
		if (solution)
		{
			ImportProjectDialog importDialog;
			importDialog.create(this, L"Select project(s)", solution);

			if (importDialog.showModal() == ui::DrOk)
			{
				RefArray< Project > externalProjects;
				importDialog.getSelectedProjects(externalProjects);

				for (RefArray< Project >::iterator i = externalProjects.begin(); i != externalProjects.end(); ++i)
					m_project->addDependency(gc_new< ExternalDependency >(filePath.getPathName(), (*i)->getName()));

				updateDependencyList();
			}

			importDialog.destroy();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
}
