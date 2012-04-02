#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/App/GeneralSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.GeneralSettingsPage", 0, GeneralSettingsPage, ISettingsPage)

bool GeneralSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4)))
		return false;

	Ref< ui::Static > staticDictionary = new ui::Static();
	staticDictionary->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_DICTIONARY"));

	m_editDictionary = new ui::Edit();
	m_editDictionary->create(containerInner, settings->getProperty< PropertyString >(L"Editor.Dictionary"));

	m_checkAutoSave = new ui::CheckBox();
	m_checkAutoSave->create(container, i18n::Text(L"EDITOR_SETTINGS_AUTOSAVE"));
	m_checkAutoSave->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.AutoSave"));

	m_checkBuildWhenSourceModified = new ui::CheckBox();
	m_checkBuildWhenSourceModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_SOURCE_MODIFIED"));
	m_checkBuildWhenSourceModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified"));

	m_checkBuildWhenAssetModified = new ui::CheckBox();
	m_checkBuildWhenAssetModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_ASSET_MODIFIED"));
	m_checkBuildWhenAssetModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified"));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_GENERAL"));
	return true;
}

void GeneralSettingsPage::destroy()
{
}

bool GeneralSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyString >(L"Editor.Dictionary", m_editDictionary->getText());
	settings->setProperty< PropertyBoolean >(L"Editor.AutoSave", m_checkAutoSave->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenSourceModified", m_checkBuildWhenSourceModified->isChecked());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenAssetModified", m_checkBuildWhenAssetModified->isChecked());
	return true;
}

	}
}
