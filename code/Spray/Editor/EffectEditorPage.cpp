#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberName.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Mesh/MeshFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/Image/ImageProcessFactory.h"
#include "Render/Image/ImageProcessData.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/AudioSystem.h"
#include "Sound/SoundFactory.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/SourceData.h"
#include "Spray/Editor/ClipboardData.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Editor/EffectPreviewControl.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/Sequencer/CursorMoveEvent.h"
#include "Ui/Sequencer/KeyMoveEvent.h"
#include "Ui/Sequencer/Marker.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequenceButtonClickEvent.h"
#include "Ui/Sequencer/SequenceMovedEvent.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Range.h"
#include "Ui/Sequencer/Tick.h"
#include "Ui/Splitter.h"

// Resources
#include "Resources/BrowseBackground.h"
#include "Resources/BrowseImageProcess.h"
#include "Resources/LayerDelete.h"
#include "Resources/LayerHidden.h"
#include "Resources/LayerVisible.h"
#include "Resources/Playback.h"
#include "Resources/Randomize.h"
#include "Resources/ToggleGroundClip.h"
#include "Resources/ToggleGuideLines.h"
#include "Resources/ToggleMoveEmitter.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

class SequenceDataKey : public Object
{
	T_RTTI_CLASS;

public:
	SequenceDataKey(SequenceData* sequenceData, int32_t key)
	:	m_sequenceData(sequenceData)
	,	m_key(key)
	{
	}

	SequenceData* getSequenceData() { return m_sequenceData; }

	int32_t getKeyIndex() const { return m_key; }

private:
	Ref< SequenceData > m_sequenceData;
	int32_t m_key;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceDataKey", SequenceDataKey, Object)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEditorPage", EffectEditorPage, editor::IEditorPage)

EffectEditorPage::EffectEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_velocityVisible(false)
,	m_guideVisible(true)
,	m_moveEmitter(false)
,	m_groundClip(false)
{
}

bool EffectEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	m_audioSystem = m_editor->getStoreObject< sound::AudioSystem >(L"AudioSystem");

	Ref< db::Database > database = m_editor->getOutputDatabase();
	T_ASSERT(database);

	m_resourceManager = new resource::ResourceManager(database, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new mesh::MeshFactory(renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new render::SequenceTextureFactory());
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	m_resourceManager->addFactory(new sound::SoundFactory());
	m_resourceManager->addFactory(new render::ImageProcessFactory());
	m_resourceManager->addFactory(new EffectFactory(nullptr));

	m_effectData = m_document->getObject< EffectData >(0);
	if (!m_effectData)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolToggleGuide = new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_GUIDE"), 6, ui::Command(L"Effect.Editor.ToggleGuide"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleMove = new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_MOVE"), 7, ui::Command(L"Effect.Editor.ToggleMove"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleGroundClip = new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_GROUND_CLIP"), 8, ui::Command(L"Effect.Editor.ToggleGroundClip"), ui::ToolBarButton::BsDefaultToggle);

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	T_ASSERT(settings);

	m_guideVisible = settings->getProperty< bool >(L"EffectEditor.ToggleGuide", m_guideVisible);
	m_toolToggleGuide->setToggled(m_guideVisible);

	m_moveEmitter = settings->getProperty< bool >(L"EffectEditor.ToggleMove", m_moveEmitter);
	m_toolToggleMove->setToggled(m_moveEmitter);

	m_groundClip = settings->getProperty< bool >(L"EffectEditor.ToggleGroundClip", m_groundClip);
	m_toolToggleGroundClip->setToggled(m_groundClip);

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.Playback"), 6);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.ToggleGuideLines"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.ToggleMoveEmitter"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.ToggleGroundClip"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.BrowseBackground"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.BrowseImageProcess"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Spray.Randomize"), 1);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_REWIND"), 0, ui::Command(L"Effect.Editor.Rewind")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_PLAY"), 1, ui::Command(L"Effect.Editor.Play")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_STOP"), 2, ui::Command(L"Effect.Editor.Stop")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(m_toolToggleMove);
	m_toolBar->addItem(m_toolToggleGroundClip);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_RANDOMIZE_SEED"), 11, ui::Command(L"Effect.Editor.RandomizeSeed")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_BROWSE_BACKGROUND"), 9, ui::Command(L"Effect.Editor.BrowseBackground")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_BROWSE_POSTPROCESS"), 10, ui::Command(L"Effect.Editor.BrowseImageProcess")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EffectEditorPage::eventToolClick);

	m_previewControl = new EffectPreviewControl(m_editor);
	m_previewControl->create(container, ui::WsNone, m_resourceManager, renderSystem, m_audioSystem);
	m_previewControl->showGuide(m_guideVisible);
	m_previewControl->setMoveEmitter(m_moveEmitter);
	m_previewControl->setGroundClip(m_groundClip);

	m_sequencer = new ui::SequencerControl();
	m_sequencer->create(parent, ui::WsDoubleBuffer);
	m_sequencer->setText(i18n::Text(L"EFFECT_EDITOR_SEQUENCER"));
	m_sequencer->addEventHandler< ui::SelectionChangeEvent >(this, &EffectEditorPage::eventSequencerLayerSelect);
	m_sequencer->addEventHandler< ui::CursorMoveEvent >(this, &EffectEditorPage::eventSequencerTimeCursorMove);
	m_sequencer->addEventHandler< ui::SequenceMovedEvent >(this, &EffectEditorPage::eventSequencerLayerRearranged);
	m_sequencer->addEventHandler< ui::KeyMoveEvent >(this, &EffectEditorPage::eventSequencerKeyMove);
	m_sequencer->addEventHandler< ui::SequenceButtonClickEvent >(this, &EffectEditorPage::eventSequencerLayerClick);
	m_sequencer->addEventHandler< ui::MouseButtonDownEvent >(this, &EffectEditorPage::eventSequencerButtonDown);

	m_site->createAdditionalPanel(m_sequencer, ui::dpi96(140), true);

	m_popupMenu = new ui::Menu();
	m_popupMenu->add(new ui::MenuItem(ui::Command(L"Effect.Editor.ReplaceEmitterSource"), i18n::Text(L"EFFECT_EDITOR_REPLACE_EMITTER_SOURCE")));

	m_site->setPropertyObject(m_effectData);

	updateSequencer();
	updateEffectPreview();

	return true;
}

void EffectEditorPage::destroy()
{
	Ref< PropertyGroup > settings = m_editor->checkoutGlobalSettings();
	T_ASSERT(settings);

	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleGuide", m_guideVisible);
	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleMove", m_moveEmitter);
	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleGroundClip", m_groundClip);

	m_editor->commitGlobalSettings();
	m_audioSystem = nullptr;

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_sequencer);

	// Destroy widgets.
	safeDestroy(m_sequencer);
	safeDestroy(m_previewControl);
	safeDestroy(m_resourceManager);
}

bool EffectEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool EffectEditorPage::handleCommand(const ui::Command& command)
{
	if (!m_previewControl)
		return false;

	if (command == L"Editor.SettingsChanged")
	{
		m_previewControl->updateSettings();
		m_previewControl->update();
	}
	else if (command == L"Effect.Editor.Reset")
	{
		m_previewControl->setTotalTime(0.0f);
		m_previewControl->syncEffect();
	}
	else if (command == L"Effect.Editor.Rewind")
	{
		m_previewControl->setTotalTime(0.0f);
		m_previewControl->syncEffect();
	}
	else if (command == L"Effect.Editor.Play")
		m_previewControl->setTimeScale(1.0f);
	else if (command == L"Effect.Editor.Stop")
		m_previewControl->setTimeScale(0.0f);
	else if (command == L"Effect.Editor.ToggleGuide")
	{
		m_guideVisible = !m_guideVisible;
		m_previewControl->showGuide(m_guideVisible);
		m_toolToggleGuide->setToggled(m_guideVisible);
	}
	else if (command == L"Effect.Editor.ToggleMove")
	{
		m_moveEmitter = !m_moveEmitter;
		m_previewControl->setMoveEmitter(m_moveEmitter);
		m_previewControl->syncEffect();
		m_toolToggleMove->setToggled(m_moveEmitter);
	}
	else if (command == L"Effect.Editor.ToggleGroundClip")
	{
		m_groundClip = !m_groundClip;
		m_previewControl->setGroundClip(m_groundClip);
		m_toolToggleGroundClip->setToggled(m_groundClip);
	}
	else if (command == L"Effect.Editor.ToggleVelocity")
	{
		m_velocityVisible = !m_velocityVisible;
		m_previewControl->showVelocity(m_velocityVisible);
	}
	else if (command == L"Effect.Editor.RandomizeSeed")
	{
		m_previewControl->randomizeSeed();
		m_previewControl->syncEffect();
	}
	else if (command == L"Effect.Editor.BrowseBackground")
	{
		Ref< db::Instance > textureInstance = m_editor->browseInstance(type_of< render::ITexture >());
		if (textureInstance)
		{
			m_editor->buildAsset(textureInstance->getGuid(), false);
			m_previewControl->setBackground(resource::Id< render::ISimpleTexture >(textureInstance->getGuid()));
		}
	}
	else if (command == L"Effect.Editor.BrowseImageProcess")
	{
		Ref< db::Instance > postProcessInstance = m_editor->browseInstance(type_of< render::ImageProcessData >());
		if (postProcessInstance)
		{
			m_editor->buildAsset(postProcessInstance->getGuid(), false);
			m_previewControl->setImageProcess(resource::Id< render::ImageProcessData >(postProcessInstance->getGuid()));
		}
	}
	else if (command == L"Effect.Editor.ReplaceEmitterSource")
	{
		const TypeInfo* sourceType = m_editor->browseType(makeTypeInfoSet< SourceData >(), false, true);
		if (sourceType)
		{
			RefArray< ui::SequenceItem > selectedItems;
			if (m_sequencer->getSequenceItems(selectedItems, ui::SequencerControl::GfSelectedOnly) > 0)
			{
				for (auto selectedItem : selectedItems)
				{
					Ref< EffectLayerData > layer = selectedItem->getData< EffectLayerData >(L"LAYERDATA");

					EmitterData* emitter = layer->getEmitter();
					if (!emitter)
						continue;

					Ref< SourceData > source = mandatory_non_null_type_cast< SourceData* >(sourceType->createInstance());

					const SourceData* oldSource = emitter->getSource();
					if (oldSource)
					{
						// Extract parameters from old source and insert into new source.
						Ref< Reflection > sourceReflection = Reflection::create(source);
						T_ASSERT(sourceReflection);

						Ref< Reflection > oldSourceReflection = Reflection::create(oldSource);
						T_ASSERT(oldSourceReflection);

						for (uint32_t i = 0; i < sourceReflection->getMemberCount(); ++i)
						{
							ReflectionMember* sourceMember = sourceReflection->getMember(i);
							ReflectionMember* oldSourceMember = oldSourceReflection->findMember(RfpMemberName(sourceMember->getName()));
							if (oldSourceMember)
								sourceMember->replace(oldSourceMember);
						}

						sourceReflection->apply(source);
					}

					emitter->setSource(source);
				}
			}

			updateEffectPreview();
		}
	}
	else if (command == L"Editor.PropertiesChanging")
	{
		m_document->push();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		updateSequencer();
		updateEffectPreview();
	}
	else if (command == L"Editor.Copy")
	{
		RefArray< ui::SequenceItem > selectedItems;
		if (m_sequencer->getSequenceItems(selectedItems, ui::SequencerControl::GfSelectedOnly) > 0)
		{
			Ref< ClipboardData > clipboardData = new ClipboardData();
			for (auto selectedItem : selectedItems)
			{
				Ref< EffectLayerData > layer = selectedItem->getData< EffectLayerData >(L"LAYERDATA");
				T_ASSERT(layer);

				clipboardData->addLayer(layer);
			}
			ui::Application::getInstance()->getClipboard()->setObject(clipboardData);
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< ClipboardData > clipboardData = dynamic_type_cast< ClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (clipboardData)
		{
			for (auto layer : clipboardData->getLayers())
				m_effectData->addLayer(DeepClone(layer).create< EffectLayerData >());

			updateSequencer();
			updateEffectPreview();
		}
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_effectData = m_document->getObject< EffectData >(0);
			T_ASSERT(m_effectData);

			m_site->setPropertyObject(m_effectData);

			updateSequencer();
			updateEffectPreview();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_effectData = m_document->getObject< EffectData >(0);
			T_ASSERT(m_effectData);

			m_site->setPropertyObject(m_effectData);

			updateSequencer();
			updateEffectPreview();
		}
	}

	return true;
}

void EffectEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void EffectEditorPage::updateEffectPreview()
{
	if (m_resourceManager)
	{
		RefArray< EffectLayer > effectLayers;

		// Create effect layers.
		RefArray< ui::SequenceItem > sequencerLayers;
		m_sequencer->getSequenceItems(sequencerLayers, ui::SequencerControl::GfDefault);
		for (auto sequencerLayer : sequencerLayers)
		{
			ui::Sequence* layerItem = checked_type_cast< ui::Sequence*, false >(sequencerLayer);
			if (!layerItem->getButtonState(0))
			{
				EffectLayerData* effectLayerData = layerItem->getData< EffectLayerData >(L"LAYERDATA");
				T_ASSERT(effectLayerData);

				Ref< EffectLayer > effectLayer = effectLayerData->createEffectLayer(m_resourceManager, 0);
				if (effectLayer)
					effectLayers.push_back(effectLayer);

				layerItem->setData(L"LAYER", effectLayer);
			}
			else
				layerItem->setData(L"LAYER", nullptr);
		}

		// Create effect.
		Ref< Effect > effect = new Effect(
			m_effectData->getDuration(),
			m_effectData->getLoopStart(),
			m_effectData->getLoopEnd(),
			effectLayers
		);

		m_previewControl->setEffect(m_effectData, effect);

		float time = m_sequencer->getCursor() / 1000.0f;
		m_previewControl->setTotalTime(time);
		m_previewControl->syncEffect();
	}
	else
		m_previewControl->setEffect(0, 0);
}

void EffectEditorPage::updateSequencer()
{
	// Get map of layer visibility.
	std::map< const EffectLayerData*, bool > visibleStates;

	RefArray< ui::SequenceItem > sequencerLayers;
	m_sequencer->getSequenceItems(sequencerLayers, ui::SequencerControl::GfDefault);
	for (auto sequencerLayer : sequencerLayers)
	{
		ui::Sequence* layerItem = checked_type_cast< ui::Sequence*, false >(sequencerLayer);

		EffectLayerData* effectLayerData = layerItem->getData< EffectLayerData >(L"LAYERDATA");
		T_ASSERT(effectLayerData);

		visibleStates[effectLayerData] = layerItem->getButtonState(0);
	}

	// Remember scroll offset.
	ui::Point scrollOffset = m_sequencer->getScrollOffset();

	// Remove all layers, re-created below.
	m_sequencer->removeAllSequenceItems();

	if (!m_effectData)
	{
		m_sequencer->setEnable(false);
		m_sequencer->update();
		return;
	}

	// Add each effect layers to sequencer.
	Ref< ui::IBitmap > layerDelete = new ui::StyleBitmap(L"Spray.LayerDelete");
	Ref< ui::IBitmap > layerVisible = new ui::StyleBitmap(L"Spray.LayerVisible");
	Ref< ui::IBitmap > layerHidden = new ui::StyleBitmap(L"Spray.LayerHidden");

	for (auto layer : m_effectData->getLayers())
	{
		Ref< ui::Sequence > layerItem = new ui::Sequence(layer->getName());
		layerItem->addButton(layerDelete, layerDelete, ui::Command(L"Effect.Editor.DeleteLayer"));
		layerItem->addButton(layerVisible, layerHidden, ui::Command(L"Effect.Editor.ToggleLayerVisible"), true);
		layerItem->setButtonState(0, visibleStates[layer]);
		layerItem->setData(L"LAYERDATA", layer);

		float start = layer->getTime();
		if (layer->getDuration() > 0.0f)
		{
			float end = layer->getTime() + layer->getDuration();
			Ref< ui::Range > layerRange = new ui::Range(
				int32_t(start * 1000.0f),
				int32_t(end * 1000.0f),
				true
			);
			layerRange->setData(L"LAYERDATA", layer);
			layerItem->addKey(layerRange);
		}
		else
		{
			Ref< ui::Tick > layerTick = new ui::Tick(int32_t(start * 1000.0f), true);
			layerTick->setData(L"LAYERDATA", layer);
			layerItem->addKey(layerTick);
		}

		// Add sequence entries.
		Ref< SequenceData > sequence = layer->getSequence();
		if (sequence)
		{
			const auto& keys = sequence->getKeys();
			for (int32_t j = 0; j < (int32_t)keys.size(); ++j)
			{
				Ref< ui::Marker > sequenceMarker = new ui::Marker(int32_t(keys[j].T * 1000.0f), true);
				sequenceMarker->setData(L"SEQUENCE", new SequenceDataKey(sequence, j));
				sequenceMarker->setData(L"LAYERDATA", layer);
				layerItem->addKey(sequenceMarker);
			}
		}

		m_sequencer->addSequenceItem(layerItem);
	}

	m_sequencer->setEnable(true);
	m_sequencer->setLength(int(m_effectData->getDuration() * 1000.0f));
	m_sequencer->setScrollOffset(scrollOffset);
	m_sequencer->update();
}

void EffectEditorPage::updateProfile()
{
	RefArray< ui::SequenceItem > items;
	m_sequencer->getSequenceItems(items, ui::SequencerControl::GfDefault);

	for (RefArray< ui::SequenceItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		ui::Sequence* sequence = checked_type_cast< ui::Sequence*, false >(*i);
		Ref< EffectLayer > layer = sequence->getData< EffectLayer >(L"LAYER");
		if (layer)
		{
			uint32_t npoints = m_previewControl->getEffectLayerPoints(layer);
			sequence->setDescription(toString(npoints));
		}
		else
			sequence->setDescription(L"");
	}

	m_sequencer->update();
}

void EffectEditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void EffectEditorPage::eventSequencerLayerSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::SequenceItem > selectedItems;
	if (m_sequencer->getSequenceItems(selectedItems, ui::SequencerControl::GfSelectedOnly) == 1)
	{
		ui::Sequence* selectedSequence = checked_type_cast< ui::Sequence*, false >(selectedItems.front());

		Ref< EffectLayerData > layer = selectedSequence->getData< EffectLayerData >(L"LAYERDATA");
		T_ASSERT(layer);

		ui::Marker* selectedMarker = dynamic_type_cast< ui::Marker* >(selectedSequence->getSelectedKey());
		if (selectedMarker)
		{
			// FIXME
			m_site->setPropertyObject(layer);
		}
		else
		{
			m_site->setPropertyObject(layer);
		}
	}
	else
		m_site->setPropertyObject(m_effectData);
}

void EffectEditorPage::eventSequencerTimeCursorMove(ui::CursorMoveEvent* event)
{
	float time = m_sequencer->getCursor() / 1000.0f;
	m_previewControl->setTimeScale(0.0f);
	m_previewControl->setTotalTime(time);
	m_previewControl->syncEffect();
	updateProfile();
}

void EffectEditorPage::eventSequencerLayerRearranged(ui::SequenceMovedEvent* event)
{
	Ref< EffectLayerData > movedLayer = event->getItem()->getData< EffectLayerData >(L"LAYERDATA");
	T_ASSERT(movedLayer);

	RefArray< EffectLayerData > layers = m_effectData->getLayers();

	layers.remove(movedLayer);
	layers.insert(layers.begin() + event->getMovedTo(), movedLayer);

	m_effectData->setLayers(layers);
	updateEffectPreview();
}

void EffectEditorPage::eventSequencerKeyMove(ui::KeyMoveEvent* event)
{
	ui::Range* movedRange = dynamic_type_cast< ui::Range* >(event->getKey());
	if (movedRange)
	{
		Ref< EffectLayerData > layer = movedRange->getData< EffectLayerData >(L"LAYERDATA");
		T_ASSERT(layer);

		float start = movedRange->getStart() / 1000.0f;
		float end = movedRange->getEnd() / 1000.0f;
		m_document->push();
		layer->setTime(start);
		layer->setDuration(end - start);
		m_site->setPropertyObject(layer);
	}

	ui::Tick* movedTick = dynamic_type_cast< ui::Tick* >(event->getKey());
	if (movedTick)
	{
		Ref< EffectLayerData > layer = movedTick->getData< EffectLayerData >(L"LAYERDATA");
		T_ASSERT(layer);

		float start = movedTick->getTime() / 1000.0f;
		m_document->push();
		layer->setTime(start);
		m_site->setPropertyObject(layer);
	}

	ui::Marker* movedMarker = dynamic_type_cast< ui::Marker* >(event->getKey());
	if (movedMarker)
	{
		Ref< EffectLayerData > layer = movedMarker->getData< EffectLayerData >(L"LAYERDATA");
		T_ASSERT(layer);

		Ref< SequenceDataKey > sequenceDataKey = movedMarker->getData< SequenceDataKey >(L"SEQUENCE");
		T_ASSERT(sequenceDataKey);

		SequenceData* sequenceData = sequenceDataKey->getSequenceData();
		T_ASSERT(sequenceData);

		m_document->push();

		auto& keys = sequenceData->getKeys();
		keys[sequenceDataKey->getKeyIndex()].T = float(movedMarker->getTime() / 1000.0f);

		m_site->setPropertyObject(layer);
	}

	updateEffectPreview();
}

void EffectEditorPage::eventSequencerLayerClick(ui::SequenceButtonClickEvent* event)
{
	if (event->getCommand() == L"Effect.Editor.DeleteLayer")
	{
		m_document->push();
		RefArray< EffectLayerData > layers = m_effectData->getLayers();
		layers.remove(event->getSequence()->getData< EffectLayerData >(L"LAYERDATA"));
		m_effectData->setLayers(layers);
		updateSequencer();
		updateEffectPreview();
	}
	else if (event->getCommand() == L"Effect.Editor.ToggleLayerVisible")
		updateEffectPreview();
}

void EffectEditorPage::eventSequencerButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		const ui::MenuItem* selectedItem = m_popupMenu->showModal(m_sequencer, event->getPosition());
		if (selectedItem != nullptr)
			handleCommand(selectedItem->getCommand());
	}
}

	}
}
