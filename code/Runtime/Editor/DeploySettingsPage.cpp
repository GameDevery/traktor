#include "Runtime/Editor/DeploySettingsPage.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,*,*,*,100%", 0, 4)))
		return false;

	m_checkInheritCache = new ui::CheckBox();
	m_checkInheritCache->create(container, L"Inherit editor cache(s)");

	bool inheritCache = settings->getProperty< bool >(L"Runtime.InheritCache", true);
	m_checkInheritCache->setChecked(inheritCache);

	m_checkHidePipeline = new ui::CheckBox();
	m_checkHidePipeline->create(container, L"Hide pipeline console");

	bool hidePipeline = settings->getProperty< bool >(L"Runtime.PipelineHidden", true);
	m_checkHidePipeline->setChecked(hidePipeline);

	m_checkUseDebugBinaries = new ui::CheckBox();
	m_checkUseDebugBinaries->create(container, L"Use debug binaries");

	bool useDebugBinaries = settings->getProperty< bool >(L"Runtime.UseDebugBinaries", false);
	m_checkUseDebugBinaries->setChecked(useDebugBinaries);

	m_checkStaticallyLinked = new ui::CheckBox();
	m_checkStaticallyLinked->create(container, L"Statically link product");

	bool staticallyLinked = settings->getProperty< bool >(L"Runtime.StaticallyLinked", false);
	m_checkStaticallyLinked->setChecked(staticallyLinked);

	m_checkVerboseResourceManager = new ui::CheckBox();
	m_checkVerboseResourceManager->create(container, L"Verbose resource manager");

	bool verboseResourceManager = settings->getProperty< bool >(L"Runtime.VerboseResourceManager", false);
	m_checkVerboseResourceManager->setChecked(verboseResourceManager);

	Ref< ui::Container > containerAndroid = new ui::Container();
	containerAndroid->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticAndroidHome = new ui::Static();
	staticAndroidHome->create(containerAndroid, L"Android SDK home");

	m_editAndroidHome = new ui::Edit();
	m_editAndroidHome->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"));

	Ref< ui::Static > staticAndroidNdkRoot = new ui::Static();
	staticAndroidNdkRoot->create(containerAndroid, L"Android NDK root");

	m_editAndroidNdkRoot = new ui::Edit();
	m_editAndroidNdkRoot->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"));

	Ref< ui::Static > staticAndroidToolchain = new ui::Static();
	staticAndroidToolchain->create(containerAndroid, L"Android Toolchain");

	m_editAndroidToolchain = new ui::Edit();
	m_editAndroidToolchain->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));

	Ref< ui::Static > staticAndroidApiLevel = new ui::Static();
	staticAndroidApiLevel->create(containerAndroid, L"Android API level");

	m_editAndroidApiLevel = new ui::Edit();
	m_editAndroidApiLevel->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	Ref< ui::Static > staticEmscripten = new ui::Static();
	staticEmscripten->create(containerAndroid, L"Emscripten SDK");

	m_editEmscripten = new ui::Edit();
	m_editEmscripten->create(containerAndroid, settings->getProperty< std::wstring >(L"Runtime.Emscripten", L"$(EMSCRIPTEN)"));

	Ref< ui::Container > containerEnvironment = new ui::Container();
	containerEnvironment->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticEnvironment = new ui::Static();
	staticEnvironment->create(containerEnvironment, L"Environment");

	Ref< ui::GridView > gridEnvironment = new ui::GridView();
	gridEnvironment->create(containerEnvironment, ui::WsDoubleBuffer);
	gridEnvironment->addColumn(new ui::GridColumn(L"Name", ui::dpi96(200)));
	gridEnvironment->addColumn(new ui::GridColumn(L"Value", ui::dpi96(400)));

	Ref< PropertyGroup > settingsEnvironment = settings->getProperty< PropertyGroup >(L"Runtime.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				Ref< ui::GridRow > row = new ui::GridRow();
				row->add(new ui::GridItem(i->first));
				row->add(new ui::GridItem(PropertyString::get(value)));
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

bool DeploySettingsPage::apply(PropertyGroup* settings)
{
	bool inheritCache = m_checkInheritCache->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.InheritCache", inheritCache);

	bool hidePipeline = m_checkHidePipeline->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.PipelineHidden", hidePipeline);

	bool useDebugBinaries = m_checkUseDebugBinaries->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.UseDebugBinaries", useDebugBinaries);

	bool staticallyLinked = m_checkStaticallyLinked->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.StaticallyLinked", staticallyLinked);

	bool verboseResourceManager = m_checkVerboseResourceManager->isChecked();
	settings->setProperty< PropertyBoolean >(L"Runtime.VerboseResourceManager", verboseResourceManager);

	std::wstring androidHome = m_editAndroidHome->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidHome", androidHome);

	std::wstring androidNdkRoot = m_editAndroidNdkRoot->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidNdkRoot", androidNdkRoot);

	std::wstring androidToolchain = m_editAndroidToolchain->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidToolchain", androidToolchain);

	std::wstring androidApiLevel = m_editAndroidApiLevel->getText();
	settings->setProperty< PropertyString >(L"Runtime.AndroidApiLevel", androidApiLevel);

	std::wstring emscripten = m_editEmscripten->getText();
	settings->setProperty< PropertyString >(L"Runtime.Emscripten", emscripten);

	return true;
}

	}
}
