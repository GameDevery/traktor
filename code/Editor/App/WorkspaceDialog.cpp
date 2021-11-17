#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/App/WorkspaceDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.WorkspaceDialog", WorkspaceDialog, ui::ConfigDialog)

bool WorkspaceDialog::create(ui::Widget* parent, PropertyGroup* settings)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"EDITOR_WORKSPACE_TITLE"),
		ui::dpi96(500),
		ui::dpi96(200),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &WorkspaceDialog::eventDialogClick);

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 8, 4)))
		return false;

	Ref< ui::Static > staticSystemRoot = new ui::Static();
	staticSystemRoot->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_SYSTEM_ROOT"));

	m_editSystemRoot = new ui::Edit();
	m_editSystemRoot->create(containerInner, settings->getProperty< std::wstring >(L"Runtime.SystemRoot"));

	Ref< ui::Static > staticSourceDatabase = new ui::Static();
	staticSourceDatabase->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_SOURCE_DATABASE"));

	m_editSourceDatabase = new ui::Edit();
	m_editSourceDatabase->create(containerInner, settings->getProperty< std::wstring >(L"Editor.SourceDatabase", L""));

	Ref< ui::Static > staticOutputDatabase = new ui::Static();
	staticOutputDatabase->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_OUTPUT_DATABASE"));

	m_editOutputDatabase = new ui::Edit();
	m_editOutputDatabase->create(containerInner, settings->getProperty< std::wstring >(L"Editor.OutputDatabase", L""));

	Ref< ui::Static > staticAssetPath = new ui::Static();
	staticAssetPath->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_ASSET_PATH"));

	m_editAssetPath = new ui::Edit();
	m_editAssetPath->create(containerInner, settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L""));

	m_settings = settings;

	fit(Container::FaVertical);
	return true;
}

void WorkspaceDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if (event->getCommand() == ui::DrOk)
	{
		m_settings->setProperty< PropertyString >(L"Runtime.SystemRoot", m_editSystemRoot->getText());
		m_settings->setProperty< PropertyString >(L"Editor.SourceDatabase", m_editSourceDatabase->getText());
		m_settings->setProperty< PropertyString >(L"Editor.OutputDatabase", m_editOutputDatabase->getText());
		m_settings->setProperty< PropertyString >(L"Pipeline.AssetPath", m_editAssetPath->getText());
	}
}

	}
}
