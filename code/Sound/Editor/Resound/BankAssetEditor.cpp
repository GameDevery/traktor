/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Sound/AudioResourceFactory.h"
#include "Sound/Sound.h"
#include "Sound/Player/SoundHandle.h"
#include "Sound/Player/SoundPlayer.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/GrainFactory.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/BankControl.h"
#include "Sound/Editor/Resound/BankControlGrain.h"
#include "Sound/Editor/Resound/BlendGrainFacade.h"
#include "Sound/Editor/Resound/EnvelopeGrainFacade.h"
#include "Sound/Editor/Resound/InLoopOutGrainFacade.h"
#include "Sound/Editor/Resound/GrainProperties.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"
#include "Sound/Editor/Resound/SimultaneousGrainFacade.h"
#include "Sound/Editor/Resound/TriggerGrainFacade.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Panel.h"
#include "Ui/Splitter.h"
#include "Ui/Envelope/DefaultEnvelopeEvaluator.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::sound
{
	namespace
	{

class HandleWrapper : public Object
{
	T_RTTI_CLASS;

public:
	HandleWrapper(int32_t id) : m_id(id) {}

	int32_t get() const { return m_id; }

private:
	int32_t m_id;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankAssetEditor.HandleWrapper", HandleWrapper, Object)

class EditorGrainFactory : public GrainFactory
{
	T_RTTI_CLASS;

public:
	EditorGrainFactory(resource::IResourceManager* resourceManager, std::map< const IGrainData*, const IGrain* >& instances)
	:	GrainFactory(resourceManager)
	,	m_instances(instances)
	{
		m_instances.clear();
	}

	virtual Ref< IGrain > createInstance(const IGrainData* grainData) override
	{
		if (!grainData)
			return nullptr;

		Ref< IGrain > grain = GrainFactory::createInstance(grainData);
		m_instances.insert(std::make_pair(grainData, grain.ptr()));

		return grain;
	}

private:
	std::map< const IGrainData*, const IGrain* >& m_instances;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankAssetEditor.EditorGrainFactory", EditorGrainFactory, GrainFactory)

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BankAssetEditor", BankAssetEditor, editor::IObjectEditor)

BankAssetEditor::BankAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool BankAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< BankAsset* >(object);
	if (!m_asset)
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(parent, true, 180_ut);

	splitter->addEventHandler< ui::TimerEvent >(this, &BankAssetEditor::eventTimer);
	splitter->startTimer(30);

	Ref< ui::Splitter > splitterLeftH = new ui::Splitter();
	splitterLeftH->create(splitter, false, -150_ut);

	Ref< ui::Splitter > splitterRightH = new ui::Splitter();
	splitterRightH->create(splitter, false, -150_ut);

	Ref< ui::Panel > containerBank = new ui::Panel();
	containerBank->create(splitterLeftH, L"Grains", new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	m_toolBarItemPlay = new ui::ToolBarButton(L"Play", 0, ui::Command(L"Bank.PlayGrain"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	m_toolBarItemRepeat = new ui::ToolBarButton(L"Repeat", 0, ui::Command(L"Bank.RepeatGrain"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(containerBank);
	m_toolBar->addItem(m_toolBarItemPlay);
	m_toolBar->addItem(m_toolBarItemRepeat);
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &BankAssetEditor::eventToolBarClick);

	m_bankControl = new BankControl();
	m_bankControl->create(containerBank);
	m_bankControl->addEventHandler< ui::SelectionChangeEvent >(this, &BankAssetEditor::eventGrainSelect);
	m_bankControl->addEventHandler< ui::MouseButtonUpEvent >(this, &BankAssetEditor::eventGrainButtonUp);

	m_containerParameters = new ui::Panel();
	m_containerParameters->create(splitterLeftH, L"Parameters", new ui::TableLayout(L"*,100%", L"*", 4_ut, 0_ut));

	Ref< ui::Panel > m_containerGrainProperties = new ui::Panel();
	m_containerGrainProperties->create(splitterRightH, L"Properties", new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut));

	m_grainProperties = new GrainProperties(m_editor);
	m_grainProperties->create(m_containerGrainProperties);
	m_grainProperties->addEventHandler< ui::ContentChangeEvent >(this, &BankAssetEditor::eventGrainPropertiesChange);

	m_containerGrainView = new ui::Panel();
	m_containerGrainView->create(splitterRightH, L"View", new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut));

	m_menuGrains = new ui::Menu();
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.AddGrain"), L"Add grain..."));
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.RemoveGrain"), L"Remove grain..."));

	// Create grain editor facades.
	m_grainFacades[&type_of< BlendGrainData >()] = new BlendGrainFacade();
	m_grainFacades[&type_of< EnvelopeGrainData >()] = new EnvelopeGrainFacade();
	m_grainFacades[&type_of< InLoopOutGrainData >()] = new InLoopOutGrainFacade();
	m_grainFacades[&type_of< MuteGrainData >()] = new MuteGrainFacade();
	m_grainFacades[&type_of< PlayGrainData >()] = new PlayGrainFacade();
	m_grainFacades[&type_of< RandomGrainData >()] = new RandomGrainFacade();
	m_grainFacades[&type_of< RepeatGrainData >()] = new RepeatGrainFacade();
	m_grainFacades[&type_of< SequenceGrainData >()] = new SequenceGrainFacade();
	m_grainFacades[&type_of< SimultaneousGrainData >()] = new SimultaneousGrainFacade();
	m_grainFacades[&type_of< TriggerGrainData >()] = new TriggerGrainFacade();

	// Get sound player for preview.
	m_soundPlayer = m_editor->getObjectStore()->get< SoundPlayer >();

	m_resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new AudioResourceFactory());

	updateBankControl();
	updateProperties();
	return true;
}

void BankAssetEditor::destroy()
{
	if (m_soundHandle)
		m_soundHandle->stop();

	m_soundPlayer = nullptr;

	if (m_resourceManager)
		m_resourceManager = nullptr;
}

void BankAssetEditor::apply()
{
	m_instance->setObject(m_asset);
}

bool BankAssetEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Bank.AddGrain")
	{
		IGrainData* parentGrain = 0;
		IGrainFacade* parentGrainFacade = 0;

		BankControlGrain* selectedItem = m_bankControl->getSelected();
		if (selectedItem)
		{
			parentGrain = selectedItem->getGrain();
			T_ASSERT(parentGrain);

			parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
			if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
				return true;
		}

		const TypeInfo* grainType = m_editor->browseType(makeTypeInfoSet< IGrainData >(), false, true);
		if (grainType)
		{
			Ref< IGrainData > grain = checked_type_cast< IGrainData*, false >(grainType->createInstance());

			if (parentGrainFacade)
				parentGrainFacade->addChild(parentGrain, grain);
			else
				m_asset->addGrain(grain);

			updateBankControl();
			updateProperties();
		}
	}
	else if (command == L"Bank.RemoveGrain")
	{
		BankControlGrain* selectedItem = m_bankControl->getSelected();
		if (selectedItem)
		{
			IGrainData* grain = selectedItem->getGrain();
			T_ASSERT(grain);

			if (selectedItem->getParent())
			{
				IGrainData* parentGrain = selectedItem->getParent()->getGrain();
				T_ASSERT(parentGrain);

				IGrainFacade* parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
				if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
					return true;

				parentGrainFacade->removeChild(parentGrain, grain);
			}
			else
				m_asset->removeGrain(grain);

			updateBankControl();
			updateProperties();
		}
	}
	else if (command == L"Bank.PlayGrain")
	{
		if (!m_soundHandle || !m_soundHandle->isPlaying())
		{
			RefArray< IGrainData > grainData;
			RefArray< IGrain > grains;

			// Play only selected grain.
			BankControlGrain* selectedItem = m_bankControl->getSelected();
			if (selectedItem)
			{
				IGrainData* grain = selectedItem->getGrain();
				T_ASSERT(grain);

				grainData.push_back(grain);
			}
			else
			{
				grainData = m_asset->getGrains();
			}

			// Create grains from data.
			EditorGrainFactory grainFactory(m_resourceManager, m_grainInstances);
			for (uint32_t i = 0; i < grainData.size(); ++i)
			{
				Ref< IGrain > grain = grainFactory.createInstance(grainData[i]);
				if (grain)
					grains.push_back(grain);
			}

			// Log all grains which failed to produce an instance.
			for (std::map< const IGrainData*, const IGrain* >::const_iterator i = m_grainInstances.begin(); i != m_grainInstances.end(); ++i)
			{
				if (!i->second)
					log::warning << L"Unable to create instance of " << type_name(i->first) << Endl;
			}

			// Create playback buffer.
			if (!grains.empty())
			{
				m_bankBuffer = new BankBuffer(grains);
				m_soundHandle = m_soundPlayer->play(new Sound(m_bankBuffer, 0, 1.0f, 0.0f), 0);

				for (auto sliderParameter : m_sliderParameters)
				{
					const HandleWrapper* id = sliderParameter->getData< HandleWrapper >(L"ID");
					T_ASSERT(id);

					m_soundHandle->setParameter(
						id->get(),
						sliderParameter->getValue() / 100.0f
					);
				}
			}
			else
				log::error << L"No grains to play" << Endl;
		}
		else if (m_soundHandle && m_soundHandle->isPlaying())
		{
			m_soundHandle->stop();
			m_soundHandle = nullptr;

			for (auto graincell : m_bankControl->getGrains())
				graincell->setActive(false);
			m_bankControl->update();

			m_toolBarItemPlay->setToggled(false);
			m_toolBar->update();
		}
	}
	else if (!m_grainProperties->handleCommand(command))
		return false;

	return true;
}

void BankAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

ui::Size BankAssetEditor::getPreferredSize() const
{
	return ui::Size(800, 600);
}

void BankAssetEditor::updateBankControl(BankControlGrain* parent, const RefArray< IGrainData >& grains)
{
	for (auto grain : grains)
	{
		if (!grain)
			continue;

		IGrainFacade* grainFacade = m_grainFacades[&type_of(grain)];
		if (!grainFacade)
			continue;

		Ref< BankControlGrain > item = new BankControlGrain(
			parent,
			grain,
			grainFacade->getText(grain),
			grainFacade->getImage(grain)
		);
		m_bankControl->add(item);

		RefArray< IGrainData > childGrains;
		if (grainFacade->getChildren(grain, childGrains))
			updateBankControl(item, childGrains);
	}
}

void BankAssetEditor::updateBankControl()
{
	m_bankControl->removeAll();

	const RefArray< IGrainData >& grains = m_asset->getGrains();
	updateBankControl(0, grains);

	m_bankControl->update();
}

void BankAssetEditor::updateProperties()
{
	std::set< std::wstring > properties;

	// Scan grains for all dynamic properties.
	RefArray< IGrainData > grains = m_asset->getGrains();
	while (!grains.empty())
	{
		Ref< IGrainData > grain = grains.front();
		grains.pop_front();

		if (!grain)
			continue;

		IGrainFacade* grainFacade = m_grainFacades[&type_of(grain)];
		if (grainFacade)
		{
			grainFacade->getProperties(grain, properties);

			RefArray< IGrainData > childGrains;
			if (grainFacade->getChildren(grain, childGrains))
				grains.insert(grains.end(), childGrains.begin(), childGrains.end());
		}
	}

	// Destroy previous sliders.
	m_sliderParameters.clear();
	while (m_containerParameters->getLastChild())
		m_containerParameters->getLastChild()->destroy();

	// Create slider for each dynamic property.
	for (const auto& property : properties)
	{
		if (property.empty())
			continue;

		Ref< ui::Static > staticParameter = new ui::Static();
		staticParameter->create(m_containerParameters, property);

		Ref< ui::Slider > sliderParameter = new ui::Slider();
		sliderParameter->create(m_containerParameters);
		sliderParameter->setRange(0, 100);
		sliderParameter->addEventHandler< ui::ContentChangeEvent >(this, &BankAssetEditor::eventParameterChange);
		sliderParameter->setData(L"ID", new HandleWrapper(
			getParameterHandle(property)
		));

		m_sliderParameters.push_back(sliderParameter);
	}

	m_containerParameters->update();
}

void BankAssetEditor::eventParameterChange(ui::ContentChangeEvent* event)
{
	const ui::Slider* slider = checked_type_cast< const ui::Slider*, false >(event->getSender());
	const HandleWrapper* id = slider->getData< HandleWrapper >(L"ID");
	T_ASSERT(id);

	if (m_soundHandle)
		m_soundHandle->setParameter(id->get(), slider->getValue() / 100.0f);
}

void BankAssetEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void BankAssetEditor::eventGrainSelect(ui::SelectionChangeEvent* event)
{
	BankControlGrain* item = checked_type_cast< BankControlGrain* >(event->getItem());
	if (item)
	{
		safeDestroy(m_currentGrainView);
		m_grainProperties->set(item->getGrain());

		IGrainFacade* grainFacade = m_grainFacades[&type_of(item->getGrain())];
		if (grainFacade)
			m_currentGrainView = grainFacade->createView(item->getGrain(), m_containerGrainView);

		if (m_currentGrainView)
			m_currentGrainView->addEventHandler< ui::ContentChangeEvent >(this, &BankAssetEditor::eventGrainViewChange);
	}
	else
	{
		safeDestroy(m_currentGrainView);
		m_grainProperties->set(0);
	}
	m_containerGrainView->update();
}

void BankAssetEditor::eventGrainButtonUp(ui::MouseButtonUpEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		const ui::MenuItem* selectedItem = m_menuGrains->showModal(m_bankControl, event->getPosition());
		if (selectedItem)
			handleCommand(selectedItem->getCommand());
		event->consume();
	}
}

void BankAssetEditor::eventGrainPropertiesChange(ui::ContentChangeEvent* event)
{
	// Stop playing if properties has changed, need to reflect changes
	// without interference otherwise filter instances will be incorrect.
	if (m_soundHandle && m_bankBuffer)
	{
		IAudioBufferCursor* cursor = m_soundHandle->getCursor();
		if (cursor)
			m_bankBuffer->updateCursor(cursor);
	}

	if (m_currentGrainView)
		m_currentGrainView->update();

	updateBankControl();
	updateProperties();
}

void BankAssetEditor::eventGrainViewChange(ui::ContentChangeEvent* event)
{
	// Stop playing if properties has changed, need to reflect changes
	// without interference otherwise filter instances will be incorrect.
	if (m_soundHandle && m_bankBuffer)
	{
		IAudioBufferCursor* cursor = m_soundHandle->getCursor();
		if (cursor)
			m_bankBuffer->updateCursor(cursor);
	}

	// \fixme
	// m_grainProperties->reset();
}

void BankAssetEditor::eventTimer(ui::TimerEvent* event)
{
	if (!m_soundHandle)
		return;

	if (
		!m_soundHandle->isPlaying() &&
		m_toolBarItemPlay->isToggled()
	)
	{
		if (!m_toolBarItemRepeat->isToggled())
		{
			m_toolBarItemPlay->setToggled(false);
			m_toolBar->update();
		}
		else
		{
			T_ASSERT(m_bankBuffer);
			m_soundHandle = m_soundPlayer->play(new Sound(m_bankBuffer, 0, 1.0f, 0.0f), 0);

			for (auto sliderParameter : m_sliderParameters)
			{
				const HandleWrapper* id = sliderParameter->getData< HandleWrapper >(L"ID");
				T_ASSERT(id);

				m_soundHandle->setParameter(
					id->get(),
					sliderParameter->getValue() / 100.0f
				);
			}
		}
	}

	if (m_bankBuffer && m_soundHandle->isPlaying())
	{
		IAudioBufferCursor* cursor = m_soundHandle->getCursor();
		if (cursor)
		{
			RefArray< const IGrain > activeGrains;
			m_bankBuffer->getActiveGrains(cursor, activeGrains);

			for (auto grainCell : m_bankControl->getGrains())
			{
				const IGrainData* grainData = grainCell->getGrain();
				const IGrain* grain = m_grainInstances[grainData];

				bool isActive = false;
				if (grain)
				{
					for (auto activeGrain : activeGrains)
					{
						if (activeGrain == grain)
						{
							isActive = true;
							break;
						}
					}
				}
				grainCell->setActive(isActive);
			}

			m_bankControl->update();
		}
	}
}

}
