#include "Editor/App/DefaultPropertiesView.h"
#include "Editor/App/EditorPageSite.h"
#include "Editor/App/EditorForm.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPageSite", EditorPageSite, IEditorPageSite)

EditorPageSite::EditorPageSite(EditorForm* editor, bool active)
:	m_editor(editor)
,	m_active(active)
{
}

void EditorPageSite::show()
{
	if (m_active)
		return;

	for (auto i = m_panelWidgets.begin(); i != m_panelWidgets.end(); ++i)
	{
		if (i->second)
			m_editor->showAdditionalPanel(i->first);
	}

	m_active = true;
}

void EditorPageSite::hide()
{
	if (!m_active)
		return;

	for (auto i = m_panelWidgets.begin(); i != m_panelWidgets.end(); ++i)
	{
		i->second = i->first->isVisible(false);
		m_editor->hideAdditionalPanel(i->first);
	}

	m_active = false;
}

Ref< PropertiesView > EditorPageSite::createPropertiesView(ui::Widget* parent)
{
	Ref< DefaultPropertiesView > propertiesView = new DefaultPropertiesView(m_editor);
	if (propertiesView->create(parent))
		return propertiesView;
	else
		return nullptr;
}

void EditorPageSite::createAdditionalPanel(ui::Widget* widget, int size, bool south)
{
	m_panelWidgets.insert(std::make_pair(widget, widget->isVisible(true)));

	if (!m_active)
		widget->hide();

	m_editor->createAdditionalPanel(widget, size, south ? 0 : 1);
}

void EditorPageSite::destroyAdditionalPanel(ui::Widget* widget)
{
	m_panelWidgets.erase(widget);
	m_editor->destroyAdditionalPanel(widget);
}

void EditorPageSite::showAdditionalPanel(ui::Widget* widget)
{
	auto i = m_panelWidgets.find(widget);
	if (i != m_panelWidgets.end())
	{
		if (m_active)
			m_editor->showAdditionalPanel(widget);
		else
			i->second = true;
	}
}

void EditorPageSite::hideAdditionalPanel(ui::Widget* widget)
{
	auto i = m_panelWidgets.find(widget);
	if (i != m_panelWidgets.end())
	{
		if (m_active)
			m_editor->hideAdditionalPanel(widget);
		else
			i->second = false;
	}
}

	}
}
