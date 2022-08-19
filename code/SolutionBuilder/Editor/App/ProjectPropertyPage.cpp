#include "Core/Io/FileSystem.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/Editor/App/ProjectPropertyPage.h"
#include "SolutionBuilder/Editor/App/ImportProjectDialog.h"
#include "Ui/Application.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/Button.h"
#include "Ui/MessageBox.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

struct DependencyPredicate
{
	bool operator () (const Dependency* dep1, const Dependency* dep2) const
	{
		return dep1->getName().compare(dep2->getName()) < 0;
	}
};

struct ProjectSortPredicate
{
	bool operator () (const Project* p1, const Project* p2) const
	{
		return p1->getName().compare(p2->getName()) < 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ProjectPropertyPage", ProjectPropertyPage, ui::Container)

bool ProjectPropertyPage::create(ui::Widget* parent)
{
	const int32_t f = ui::dpi96(4);

	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"*,100%", f, f)
	))
		return false;

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(this, L"Include project in build");
	m_checkEnable->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventEnableClick);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*,100%,*", 0, f));

	Ref< ui::Static > staticSourcePath = new ui::Static();
	staticSourcePath->create(container, L"Source path");

	m_editSourcePath = new ui::Edit();
	m_editSourcePath->create(container);
	m_editSourcePath->addEventHandler< ui::FocusEvent >(this, &ProjectPropertyPage::eventFocusSource);

	Ref< ui::Static > staticDependencies = new ui::Static();
	staticDependencies->create(container, L"Dependencies");

	m_listDependencies = new ui::GridView();
	m_listDependencies->create(container, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsMultiSelect);
	m_listDependencies->addColumn(new ui::GridColumn(L"Dependency", ui::dpi96(160)));
	m_listDependencies->addColumn(new ui::GridColumn(L"Location", ui::dpi96(200)));
	m_listDependencies->addColumn(new ui::GridColumn(L"Inherit include paths", ui::dpi96(130)));
	m_listDependencies->addColumn(new ui::GridColumn(L"Link", ui::dpi96(50)));
	m_listDependencies->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ProjectPropertyPage::eventDependencyDoubleClick);

	Ref< ui::Static > staticAvailable = new ui::Static();
	staticAvailable->create(container, L"Available");

	Ref< ui::Container > containerAvailable = new ui::Container();
	containerAvailable->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, f));

	m_dropAvailable = new ui::DropDown();
	m_dropAvailable->create(containerAvailable);

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerAvailable, L"Add");
	buttonAdd->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickAdd);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerAvailable, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickRemove);

	Ref< ui::Button > buttonAddExternal = new ui::Button();
	buttonAddExternal->create(containerAvailable, L"External...");
	buttonAddExternal->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickAddExternal);

	return true;
}

void ProjectPropertyPage::set(Solution* solution, Project* project, const std::wstring& solutionFileName)
{
	m_solution = solution;
	m_project = project;
	m_solutionFileName = solutionFileName;

	m_checkEnable->setChecked(project->getEnable());
	m_editSourcePath->setText(project->getSourcePath());

	updateDependencyList();
}

void ProjectPropertyPage::updateDependencyList()
{
	RefArray< Dependency > dependencies = m_project->getDependencies();
	const wchar_t* c_link[] = { L"No", L"Yes", L"Force" };

	// Sort all dependencies.
	dependencies.sort(DependencyPredicate());

	m_listDependencies->removeAllRows();

	// Add all local dependencies first.
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ExternalDependency >(*i))
			continue;

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem((*i)->getName()));
		row->add(new ui::GridItem((*i)->getLocation()));
		row->add(new ui::GridItem((*i)->getInheritIncludePaths() ? L"Yes" : L"No"));
		row->add(new ui::GridItem(c_link[(*i)->getLink()]));
		row->setData(L"DEPENDENCY", *i);
		m_listDependencies->addRow(row);
	}

	// Add external dependencies last.
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ProjectDependency >(*i))
			continue;

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem((*i)->getName()));
		row->add(new ui::GridItem((*i)->getLocation()));
		row->add(new ui::GridItem((*i)->getInheritIncludePaths() ? L"Yes" : L"No"));
		row->add(new ui::GridItem(c_link[(*i)->getLink()]));
		row->setData(L"DEPENDENCY", *i);
		m_listDependencies->addRow(row);
	}

	// Get available projects, remove all local projects which are already in dependency list.
	RefArray< Project > projects = m_solution->getProjects();
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (!is_a< ProjectDependency >(*i))
			continue;

		RefArray< Project >::iterator j = std::find(projects.begin(), projects.end(), static_cast< ProjectDependency* >((*i).ptr())->getProject());
		if (j != projects.end())
			projects.erase(j);
	}

	// Update drop down with available projects.
	projects.sort(ProjectSortPredicate());
	m_dropAvailable->removeAll();
	for(RefArray< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if (*i != m_project)
			m_dropAvailable->add((*i)->getName());
	}
}

void ProjectPropertyPage::eventEnableClick(ui::ButtonClickEvent* event)
{
	m_project->setEnable(m_checkEnable->isChecked());

	// Update solution tree to reflect enabled state of project.
	ui::ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

void ProjectPropertyPage::eventFocusSource(ui::FocusEvent* event)
{
	if (event->lostFocus())
		m_project->setSourcePath(m_editSourcePath->getText());
}

void ProjectPropertyPage::eventDependencyDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	Ref< Dependency > dependency = event->getRow()->getData< Dependency >(L"DEPENDENCY");
	if (!dependency)
		return;

	int32_t column = event->getColumnIndex();
	if (column < 0)
		return;

	if (column == 1)	// Location
	{
		Ref< ExternalDependency > selectedDependency = dynamic_type_cast< ExternalDependency* >(dependency);
		if (!selectedDependency)
			return;

		ui::InputDialog::Field inputFields[] =
		{
			ui::InputDialog::Field(L"Location", selectedDependency->getSolutionFileName())
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			this,
			L"External dependency",
			L"Change location",
			inputFields,
			sizeof_array(inputFields)
		);
		if (inputDialog.showModal() == ui::DialogResult::Ok)
		{
			selectedDependency->setSolutionFileName(inputFields[0].value);
			updateDependencyList();
		}
		inputDialog.destroy();
	}
	else if (column == 2)	// Inherit
	{
		bool inherit = !dependency->getInheritIncludePaths();
		dependency->setInheritIncludePaths(inherit);
		updateDependencyList();
	}
	else if (column == 3)	// Link
	{
		int32_t link = (dependency->getLink() + 1) % (Dependency::LnkForce + 1);
		dependency->setLink((Dependency::Link)link);
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickAdd(ui::ButtonClickEvent* event)
{
	std::wstring dependencyName = m_dropAvailable->getSelectedItem();
	if (!dependencyName.empty())
	{
		const RefArray< Project >& projects = m_solution->getProjects();
		for(RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			if ((*i)->getName() == dependencyName)
			{
				Ref< ProjectDependency > dependency = new ProjectDependency(*i);
				m_project->addDependency(dependency);
				break;
			}
		}
		m_dropAvailable->select(-1);
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickRemove(ui::ButtonClickEvent* event)
{
	Ref< ui::GridRow > selectedRow = m_listDependencies->getSelectedRow();
	if (!selectedRow)
		return;

	Ref< Dependency > selectedDependency = selectedRow->getData< Dependency >(L"DEPENDENCY");
	T_ASSERT(selectedDependency);

	RefArray< Dependency > dependencies = m_project->getDependencies();
	RefArray< Dependency >::iterator i = std::find(dependencies.begin(), dependencies.end(), selectedDependency);
	T_ASSERT(i != dependencies.end());

	dependencies.erase(i);
	m_project->setDependencies(dependencies);

	updateDependencyList();
}

void ProjectPropertyPage::eventClickAddExternal(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	fileDialog.create(this, type_name(this), L"Select solution", L"SolutionBuilder solutions;*.xms");

	Path filePath;
	if (fileDialog.showModal(filePath) == ui::DialogResult::Ok)
	{
		Ref< Solution > solution = SolutionLoader().load(filePath.getPathName());
		if (solution)
		{
			ImportProjectDialog importDialog;
			importDialog.create(this, L"Select project(s)", false, solution);

			if (importDialog.showModal() == ui::DialogResult::Ok)
			{
				Path filePathRel;
				if (!FileSystem::getInstance().getRelativePath(
					filePath,
					Path(m_solutionFileName).getPathOnly(),
					filePathRel
				))
					filePathRel = filePath;

				RefArray< Project > externalProjects;
				importDialog.getSelectedProjects(externalProjects);

				for (auto externalProject : externalProjects)
					m_project->addDependency(new ExternalDependency(filePathRel.getPathName(), externalProject->getName()));

				updateDependencyList();
			}

			importDialog.destroy();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
}

	}
}
