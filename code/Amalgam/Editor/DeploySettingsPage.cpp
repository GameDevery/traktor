#include "Amalgam/Editor/DeploySettingsPage.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,100%", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%", 0, 4)))
		return false;

	Ref< ui::Static > staticRemoteDatabasePort = new ui::Static();
	staticRemoteDatabasePort->create(containerInner, L"Remote database port");

	m_editRemoteDatabasePort = new ui::Edit();
	m_editRemoteDatabasePort->create(containerInner, toString(c_remoteDatabasePort), ui::WsClientBorder, new ui::NumericEditValidator(false, 0, 65535, 0));

	Ref< ui::Static > staticTargetManagerPort = new ui::Static();
	staticTargetManagerPort->create(containerInner, L"Target manager port");

	m_editTargetManagerPort = new ui::Edit();
	m_editTargetManagerPort->create(containerInner, toString(c_targetConnectionPort), ui::WsClientBorder, new ui::NumericEditValidator(false, 0, 65535, 0));

	m_checkInheritCache = new ui::CheckBox();
	m_checkInheritCache->create(container, L"Inherit editor cache(s)");

	int32_t remoteDatabasePort = settings->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort);
	m_editRemoteDatabasePort->setText(toString(remoteDatabasePort));

	int32_t targetManagerPort = settings->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort);
	m_editTargetManagerPort->setText(toString(targetManagerPort));

	bool inheritCache = settings->getProperty< PropertyBoolean >(L"Amalgam.InheritCache", true);
	m_checkInheritCache->setChecked(inheritCache);

	Ref< ui::Container > containerEnvironment = new ui::Container();
	containerEnvironment->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticEnvironment = new ui::Static();
	staticEnvironment->create(containerEnvironment, L"Environment");

	Ref< ui::custom::GridView > gridEnvironment = new ui::custom::GridView();
	gridEnvironment->create(containerEnvironment, ui::WsClientBorder | ui::WsDoubleBuffer);
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Name", 200));
	gridEnvironment->addColumn(new ui::custom::GridColumn(L"Value", 400));

	Ref< PropertyGroup > settingsEnvironment = settings->getProperty< PropertyGroup >(L"Amalgam.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
				row->add(new ui::custom::GridItem(i->first));
				row->add(new ui::custom::GridItem(PropertyString::get(value)));
				gridEnvironment->addRow(row);
			}
		}
	}

	parent->setText(L"Deploy");
	return true;
}

void DeploySettingsPage::destroy()
{
}

bool DeploySettingsPage::apply(Settings* settings)
{
	int32_t remoteDatabasePort = parseString< int32_t >(m_editRemoteDatabasePort->getText());
	settings->setProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", remoteDatabasePort);

	int32_t targetManagerPort = parseString< int32_t >(m_editTargetManagerPort->getText());
	settings->setProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", targetManagerPort);

	bool inheritCache = m_checkInheritCache->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.InheritCache", inheritCache);

	return true;
}

	}
}
