#include "Runtime/Editor/TargetEditor.h"
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/FlowLayout.h"
#include "Ui/Image.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "Ui/EditList.h"
#include "Ui/EditListEditEvent.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/MiniButton.h"
#include "Ui/Panel.h"
#include "Ui/Splitter.h"
#include "Ui/ListBox/ListBox.h"

// Resources
#include "Resources/NoIcon.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetEditor", TargetEditor, editor::IObjectEditor)

TargetEditor::TargetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool TargetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	const int32_t f = ui::dpi96(4);

	m_editInstance = instance;
	m_editTarget = checked_type_cast< Target* >(object);

	m_containerOuter = new ui::Container();
	m_containerOuter->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", f, f));

	Ref< ui::Splitter > splitterInner = new ui::Splitter();
	splitterInner->create(m_containerOuter, true, ui::dpi96(200), false, ui::dpi96(100));

	Ref< ui::Container > containerTargetConfigurations = new ui::Container();
	containerTargetConfigurations->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, f));

	m_listBoxTargetConfigurations = new ui::EditList();
	m_listBoxTargetConfigurations->create(containerTargetConfigurations, ui::ListBox::WsSingle);
	m_listBoxTargetConfigurations->addEventHandler< ui::EditListEditEvent >(this, &TargetEditor::eventListBoxTargetConfigurationsEdit);
	m_listBoxTargetConfigurations->addEventHandler< ui::SelectionChangeEvent >(this, &TargetEditor::eventListBoxTargetConfigurationsSelect);

	Ref< ui::Container > containerManageTargetConfigurations = new ui::Container();
	containerManageTargetConfigurations->create(containerTargetConfigurations, ui::WsNone, new ui::FlowLayout(0, 0, f, f));

	Ref< ui::Button > buttonNewTargetConfiguration = new ui::Button();
	buttonNewTargetConfiguration->create(containerManageTargetConfigurations, L"New...");
	buttonNewTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonNewTargetConfigurationClick);

	Ref< ui::Button > buttonCloneTargetConfiguration = new ui::Button();
	buttonCloneTargetConfiguration->create(containerManageTargetConfigurations, L"Clone");
	buttonCloneTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonCloneTargetConfigurationClick);

	Ref< ui::Button > buttonRemoveTargetConfiguration = new ui::Button();
	buttonRemoveTargetConfiguration->create(containerManageTargetConfigurations, L"Delete");
	buttonRemoveTargetConfiguration->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonRemoveTargetConfigurationClick);

	Ref< ui::Container > containerEditTargetConfiguration = new ui::Container();
	containerEditTargetConfiguration->create(splitterInner, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, f));

	Ref< ui::Panel > panelGeneral = new ui::Panel();
	panelGeneral->create(containerEditTargetConfiguration, L"General", new ui::TableLayout(L"100%,128", L"*", 2 * f, f));

	Ref< ui::Container > containerLeft = new ui::Container();
	containerLeft->create(panelGeneral, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, f));

	Ref< ui::Static > staticPlatform = new ui::Static();
	staticPlatform->create(containerLeft, L"Platform");

	m_dropDownPlatform = new ui::DropDown();
	m_dropDownPlatform->create(containerLeft);
	m_dropDownPlatform->addEventHandler< ui::SelectionChangeEvent >(this, &TargetEditor::eventDropDownPlatformSelect);

	Ref< ui::Static > staticBuildRoot = new ui::Static();
	staticBuildRoot->create(containerLeft, L"Build root");

	Ref< ui::Container > container1 = new ui::Container();
	container1->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, f));

	m_editBuildRootInstance = new ui::Edit();
	m_editBuildRootInstance->create(container1, L"", ui::Edit::WsReadOnly);
	m_editBuildRootInstance->setText(L"");

	m_buttonBuildRootInstance = new ui::MiniButton();
	m_buttonBuildRootInstance->create(container1, L"...");
	m_buttonBuildRootInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseRootButtonClick);

	Ref< ui::Static > staticStartup = new ui::Static();
	staticStartup->create(containerLeft, L"Startup");

	Ref< ui::Container > container2 = new ui::Container();
	container2->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, f));

	m_editStartupInstance = new ui::Edit();
	m_editStartupInstance->create(container2, L"", ui::Edit::WsReadOnly);
	m_editStartupInstance->setText(L"");

	m_buttonStartupInstance = new ui::MiniButton();
	m_buttonStartupInstance->create(container2, L"...");
	m_buttonStartupInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseStartupButtonClick);

	Ref< ui::Static > staticDefaultInput = new ui::Static();
	staticDefaultInput->create(containerLeft, L"Default input");

	Ref< ui::Container > container3 = new ui::Container();
	container3->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, f));

	m_editDefaultInputInstance = new ui::Edit();
	m_editDefaultInputInstance->create(container3, L"", ui::Edit::WsReadOnly);
	m_editDefaultInputInstance->setText(L"");

	m_buttonDefaultInputInstance = new ui::MiniButton();
	m_buttonDefaultInputInstance->create(container3, L"...");
	m_buttonDefaultInputInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseDefaultInputButtonClick);

	Ref< ui::Static > staticOnlineConfig = new ui::Static();
	staticOnlineConfig->create(containerLeft, L"Online configuration");

	Ref< ui::Container > container4 = new ui::Container();
	container4->create(containerLeft, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, f));

	m_editOnlineConfigInstance = new ui::Edit();
	m_editOnlineConfigInstance->create(container4, L"", ui::Edit::WsReadOnly);
	m_editOnlineConfigInstance->setText(L"");

	m_buttonOnlineConfigInstance = new ui::MiniButton();
	m_buttonOnlineConfigInstance->create(container4, L"...");
	m_buttonOnlineConfigInstance->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventBrowseOnlineConfigButtonClick);

	m_bitmapNoIcon = ui::Bitmap::load(c_ResourceNoIcon, sizeof(c_ResourceNoIcon), L"png");

	m_imageIcon = new ui::Image();
	m_imageIcon->create(panelGeneral, m_bitmapNoIcon, ui::Image::WsTransparent | ui::WsDoubleBuffer);
	m_imageIcon->addEventHandler< ui::MouseButtonDownEvent >(this, &TargetEditor::eventBrowseIconClick);

	Ref< ui::Panel > panelFeatures = new ui::Panel();
	panelFeatures->create(containerEditTargetConfiguration, L"Features", new ui::TableLayout(L"100%,*,100%", L"100%", 2 * f, f));

	Ref< ui::Container > containerAvailFeatures = new ui::Container();
	containerAvailFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, f));

	Ref< ui::Static > staticAvailFeatures = new ui::Static();
	staticAvailFeatures->create(containerAvailFeatures, L"Available");

	m_listBoxAvailFeatures = new ui::ListBox();
	m_listBoxAvailFeatures->create(containerAvailFeatures, ui::ListBox::WsMultiple | ui::WsDoubleBuffer);

	Ref< ui::Container > containerManageFeatures = new ui::Container();
	containerManageFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"*", L"*,*", 0, 0));

	Ref< ui::Button > buttonAddFeature = new ui::Button();
	buttonAddFeature->create(containerManageFeatures, L">");
	buttonAddFeature->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonAddFeatureClick);

	Ref< ui::Button > buttonRemoveFeature = new ui::Button();
	buttonRemoveFeature->create(containerManageFeatures, L"<");
	buttonRemoveFeature->addEventHandler< ui::ButtonClickEvent >(this, &TargetEditor::eventButtonRemoveFeatureClick);

	Ref< ui::Container > containerUsedFeatures = new ui::Container();
	containerUsedFeatures->create(panelFeatures, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, f));

	Ref< ui::Static > staticUsedFeatures = new ui::Static();
	staticUsedFeatures->create(containerUsedFeatures, L"Using");

	m_listBoxUsedFeatures = new ui::ListBox();
	m_listBoxUsedFeatures->create(containerUsedFeatures, ui::ListBox::WsMultiple | ui::WsDoubleBuffer);

	// Collect all available platforms.
	db::recursiveFindChildInstances(
		m_editor->getSourceDatabase()->getRootGroup(),
		db::FindInstanceByType(type_of< Platform >()),
		m_platformInstances
	);

	// Collect all available features.
	RefArray< db::Instance > featureInstances;
	db::recursiveFindChildInstances(
		m_editor->getSourceDatabase()->getRootGroup(),
		db::FindInstanceByType(type_of< Feature >()),
		featureInstances
	);

	for (auto featureInstance : featureInstances)
	{
		if (featureInstance)
		{
			EditFeature ef;
			ef.feature = featureInstance->getObject< Feature >();
			ef.featureInstance = featureInstance;
			if (ef.feature)
				m_features.push_back(ef);
			else
				log::error << L"Unable to read feature \"" << featureInstance->getName() << L"\"." << Endl;
		}
	}

	m_features.sort();

	// Add all entries of platform drop down.
	for (auto platformInstance : m_platformInstances)
	{
		m_dropDownPlatform->add(
			platformInstance->getName(),
			platformInstance
		);
	}

	updateTargetConfigurations();
	updateAvailableFeatures();
	updateUsedFeatures();
	updateRoots();

	return true;
}

void TargetEditor::destroy()
{
}

void TargetEditor::apply()
{
	m_editInstance->setObject(m_editTarget);
}

bool TargetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void TargetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size TargetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(1000),
		ui::dpi96(600)
	);
}

void TargetEditor::updateTargetConfigurations()
{
	m_listBoxTargetConfigurations->removeAll();

	int32_t selected = m_listBoxTargetConfigurations->getSelected();

	const RefArray< TargetConfiguration >& configurations = m_editTarget->getConfigurations();
	for (RefArray< TargetConfiguration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		m_listBoxTargetConfigurations->add(
			(*i)->getName(),
			*i
		);
	}

	m_listBoxTargetConfigurations->select(selected);
}

void TargetEditor::updateAvailableFeatures()
{
	m_listBoxAvailFeatures->removeAll();

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		for (const auto& feature : m_features)
		{
			if (feature.feature != 0 && feature.feature->getPlatform(targetConfiguration->getPlatform()) != nullptr)
			{
				if (targetConfiguration->haveFeature(feature.featureInstance->getGuid()))
					continue;

				m_listBoxAvailFeatures->add(feature.feature->getDescription(), feature.featureInstance);
			}
		}
	}
}

void TargetEditor::updateUsedFeatures()
{
	m_listBoxUsedFeatures->removeAll();

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		std::list< EditFeature > features;

		for (const auto& featureGuid : targetConfiguration->getFeatures())
		{
			for (const auto& feature : m_features)
			{
				if (featureGuid == feature.featureInstance->getGuid())
				{
					features.push_back(feature);
					break;
				}
			}
		}

		features.sort();

		TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
		for (const auto& feature : features)
		{
			bool validForPlatform = true;
			if (targetConfiguration)
				validForPlatform = (bool)(feature.feature->getPlatform(targetConfiguration->getPlatform()) != nullptr);

			m_listBoxUsedFeatures->add(
				feature.feature->getDescription(),
				validForPlatform ? Color4ub(0, 0, 0, 0) : Color4ub(255, 0, 0, 255),
				feature.featureInstance
			);
		}
	}
}

void TargetEditor::updateRoots()
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		if (targetConfiguration->getRoot().isNotNull())
		{
			Ref< db::Instance > rootInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getRoot());
			if (rootInstance)
				m_editBuildRootInstance->setText(rootInstance->getPath());
			else
				m_editBuildRootInstance->setText(targetConfiguration->getRoot().format());
		}
		else
			m_editBuildRootInstance->setText(L"");

		if (targetConfiguration->getStartup().isNotNull())
		{
			Ref< db::Instance > startupInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getStartup());
			if (startupInstance)
				m_editStartupInstance->setText(startupInstance->getPath());
			else
				m_editStartupInstance->setText(targetConfiguration->getStartup().format());
		}
		else
			m_editStartupInstance->setText(L"");

		if (targetConfiguration->getDefaultInput().isNotNull())
		{
			Ref< db::Instance > defaultInputInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getDefaultInput());
			if (defaultInputInstance)
				m_editDefaultInputInstance->setText(defaultInputInstance->getPath());
			else
				m_editDefaultInputInstance->setText(targetConfiguration->getDefaultInput().format());
		}
		else
			m_editDefaultInputInstance->setText(L"");

		if (targetConfiguration->getOnlineConfig().isNotNull())
		{
			Ref< db::Instance > onlineConfigInstance = m_editor->getSourceDatabase()->getInstance(targetConfiguration->getOnlineConfig());
			if (onlineConfigInstance)
				m_editOnlineConfigInstance->setText(onlineConfigInstance->getPath());
			else
				m_editOnlineConfigInstance->setText(targetConfiguration->getOnlineConfig().format());
		}
		else
			m_editOnlineConfigInstance->setText(L"");
	}
	else
	{
		m_editBuildRootInstance->setText(L"");
		m_editStartupInstance->setText(L"");
		m_editDefaultInputInstance->setText(L"");
		m_editOnlineConfigInstance->setText(L"");
	}
}

void TargetEditor::updateIcon()
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		Path projectPath = FileSystem::getInstance().getCurrentVolumeAndDirectory();
		Path iconPath = targetConfiguration->getIcon();

		Ref< drawing::Image > iconImage = drawing::Image::load(projectPath + iconPath);
		if (iconImage)
		{
			drawing::ScaleFilter scaleFilter(128, 128, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			iconImage->apply(&scaleFilter);

			m_imageIcon->setImage(new ui::Bitmap(iconImage), true);
		}
		else
			m_imageIcon->setImage(m_bitmapNoIcon, true);
	}
	else
		m_imageIcon->setImage(m_bitmapNoIcon, true);
}

void TargetEditor::selectPlatform(const Guid& platformGuid) const
{
	int c = m_dropDownPlatform->count();
	for (int i = 0; i < c; ++i)
	{
		db::Instance* platformInstance = m_dropDownPlatform->getData< db::Instance >(i);
		T_ASSERT(platformInstance);

		if (platformInstance->getGuid() == platformGuid)
		{
			m_dropDownPlatform->select(i);
			return;
		}
	}
	m_dropDownPlatform->select(-1);
}

void TargetEditor::eventListBoxTargetConfigurationsEdit(ui::EditListEditEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		targetConfiguration->setName(event->getText());
		event->consume();
	}
}

void TargetEditor::eventListBoxTargetConfigurationsSelect(ui::SelectionChangeEvent* event)
{
	updateAvailableFeatures();
	updateUsedFeatures();
	updateRoots();
	updateIcon();

	m_dropDownPlatform->select(-1);

	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (targetConfiguration)
	{
		Guid platformGuid = targetConfiguration->getPlatform();
		selectPlatform(platformGuid);
	}
}

void TargetEditor::eventButtonNewTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(L"Name", L"")
	};

	Ref< ui::InputDialog > dialogInputName = new ui::InputDialog();
	dialogInputName->create(m_containerOuter, L"Enter name", L"Enter configuration name", fields, sizeof_array(fields));
	if (dialogInputName->showModal() == ui::DrOk)
	{
		Ref< TargetConfiguration > targetConfiguration = new TargetConfiguration();
		targetConfiguration->setName(fields[0].value);

		m_editTarget->addConfiguration(targetConfiguration);

		updateTargetConfigurations();
		updateAvailableFeatures();
		updateUsedFeatures();
		updateRoots();
		updateIcon();

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonCloneTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	Ref< TargetConfiguration > targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	targetConfiguration = DeepClone(targetConfiguration).create< TargetConfiguration >();
	T_ASSERT(targetConfiguration);

	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(L"Name", targetConfiguration->getName())
	};

	Ref< ui::InputDialog > dialogInputName = new ui::InputDialog();
	dialogInputName->create(m_containerOuter, L"Enter name", L"Enter configuration name", fields, sizeof_array(fields));
	if (dialogInputName->showModal() == ui::DrOk)
	{
		targetConfiguration->setName(fields[0].value);

		m_editTarget->addConfiguration(targetConfiguration);

		updateTargetConfigurations();
		updateAvailableFeatures();
		updateUsedFeatures();
		updateRoots();
		updateIcon();

		m_listBoxTargetConfigurations->select(-1);
	}
}

void TargetEditor::eventButtonRemoveTargetConfigurationClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	m_editTarget->removeConfiguration(targetConfiguration);

	updateTargetConfigurations();
	updateAvailableFeatures();
	updateUsedFeatures();
	updateRoots();
	updateIcon();

	m_listBoxTargetConfigurations->select(-1);
}

void TargetEditor::eventDropDownPlatformSelect(ui::SelectionChangeEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	db::Instance* platformInstance = m_dropDownPlatform->getSelectedData< db::Instance >();
	T_ASSERT(platformInstance);

	targetConfiguration->setPlatform(platformInstance->getGuid());

	updateAvailableFeatures();
	updateUsedFeatures();
}

void TargetEditor::eventBrowseRootButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	if (targetConfiguration->getRoot().isNull())
	{
		Ref< db::Instance > rootInstance = m_editor->browseInstance();
		if (rootInstance)
			targetConfiguration->setRoot(rootInstance->getGuid());
	}
	else
		targetConfiguration->setRoot(Guid());

	updateRoots();
}

void TargetEditor::eventBrowseStartupButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	if (targetConfiguration->getStartup().isNull())
	{
		Ref< db::Instance > startupInstance = m_editor->browseInstance();
		if (startupInstance)
			targetConfiguration->setStartup(startupInstance->getGuid());
	}
	else
		targetConfiguration->setStartup(Guid());

	updateRoots();
}

void TargetEditor::eventBrowseDefaultInputButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	if (targetConfiguration->getDefaultInput().isNull())
	{
		Ref< db::Instance > defaultInputInstance = m_editor->browseInstance();
		if (defaultInputInstance)
			targetConfiguration->setDefaultInput(defaultInputInstance->getGuid());
	}
	else
		targetConfiguration->setDefaultInput(Guid());

	updateRoots();
}

void TargetEditor::eventBrowseOnlineConfigButtonClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	if (targetConfiguration->getOnlineConfig().isNull())
	{
		Ref< db::Instance > onlineConfigInstance = m_editor->browseInstance();
		if (onlineConfigInstance)
			targetConfiguration->setOnlineConfig(onlineConfigInstance->getGuid());
	}
	else
		targetConfiguration->setOnlineConfig(Guid());

	updateRoots();
}

void TargetEditor::eventBrowseIconClick(ui::MouseButtonDownEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	ui::FileDialog fileDialog;
	if (fileDialog.create(m_containerOuter, type_name(this), L"Select icon image", L"All files;*.*"))
	{
		Path fileName;
		if (fileDialog.showModal(fileName) == ui::DrOk)
		{
			Path projectPath = FileSystem::getInstance().getCurrentVolumeAndDirectory();

			Path relativePath;
			if (!FileSystem::getInstance().getRelativePath(fileName, projectPath, relativePath))
				relativePath = fileName;

			targetConfiguration->setIcon(relativePath.normalized().getPathName());
			updateIcon();
		}
		fileDialog.destroy();
	}
}

void TargetEditor::eventButtonAddFeatureClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	std::vector< int32_t > s;
	m_listBoxAvailFeatures->getSelected(s);

	for (std::vector< int32_t >::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		Ref< db::Instance > featureInstance = m_listBoxAvailFeatures->getData< db::Instance >(*i);
		if (featureInstance)
			targetConfiguration->addFeature(featureInstance->getGuid());
	}

	updateAvailableFeatures();
	updateUsedFeatures();
}

void TargetEditor::eventButtonRemoveFeatureClick(ui::ButtonClickEvent* event)
{
	TargetConfiguration* targetConfiguration = m_listBoxTargetConfigurations->getSelectedData< TargetConfiguration >();
	if (!targetConfiguration)
		return;

	std::vector< int32_t > s;
	m_listBoxUsedFeatures->getSelected(s);

	for (std::vector< int32_t >::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		Ref< db::Instance > featureInstance = m_listBoxUsedFeatures->getData< db::Instance >(*i);
		if (featureInstance)
			targetConfiguration->removeFeature(featureInstance->getGuid());
	}

	updateAvailableFeatures();
	updateUsedFeatures();
}

bool TargetEditor::EditFeature::operator < (const EditFeature& ef) const
{
	return compareIgnoreCase(feature->getDescription(), ef.feature->getDescription()) < 0;
}

	}
}
