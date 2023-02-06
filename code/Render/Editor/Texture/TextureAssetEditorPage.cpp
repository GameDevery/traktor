/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureAssetEditorPage.h"
#include "Render/Editor/Texture/TextureControl.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureAssetEditorPage", TextureAssetEditorPage, editor::IEditorPage)

TextureAssetEditorPage::TextureAssetEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool TextureAssetEditorPage::create(ui::Container* parent)
{
	m_asset = m_document->getObject< TextureAsset >(0);
	if (!m_asset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(container);
	toolBar->addItem(new ui::ToolBarButton(L"R", ui::Command(L"Render.Texture.Editor.ToggleR"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"G", ui::Command(L"Render.Texture.Editor.ToggleG"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"B", ui::Command(L"Render.Texture.Editor.ToggleB"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"A", ui::Command(L"Render.Texture.Editor.ToggleA"), ui::ToolBarButton::BsDefaultToggled));

	Ref< ui::Container > imageContainer = new ui::Container();
	imageContainer->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0));

	m_textureControl = new TextureControl();
	m_textureControl->create(imageContainer);

	m_statusBar = new ui::StatusBar();
	m_statusBar->create(imageContainer);
	m_statusBar->addColumn(ui::dpi96(1000));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->setPropertyObject(m_asset);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &TextureAssetEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, ui::dpi96(400), false);

	updatePreview();
	return true;
}

void TextureAssetEditorPage::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_propertiesView);

	m_asset = nullptr;
}

bool TextureAssetEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool TextureAssetEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	return false;
}

void TextureAssetEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_document->getInstance(0)->getGuid() != eventId)
		return;

	updatePreview();
}

void TextureAssetEditorPage::updatePreview()
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, m_asset->getFileName());

	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (image)
	{
		m_textureControl->setImage(image, m_asset->m_output);

		StringOutputStream ss;
		ss << image->getWidth() << L" * " << image->getHeight();

		const auto& pf = image->getPixelFormat();
		if (pf.isFloatPoint())
			ss << L", fp";
		ss << L", " << pf.getColorBits() << L" bpp";
		ss << L", " << pf.getRedBits() << L" red";
		ss << L", " << pf.getGreenBits() << L" green";
		ss << L", " << pf.getBlueBits() << L" blue";
		ss << L", " << pf.getAlphaBits() << L" alpha";

		const auto* imf = image->getImageInfo();
		ss << L", gamma " << (imf ? imf->getGamma() : 2.2f) << L" ";

		m_statusBar->setText(0, ss.str());
	}
	else
	{
		m_textureControl->setImage(nullptr, m_asset->m_output);
		m_statusBar->setText(0, L"");
	}
}

void TextureAssetEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	updatePreview();
}

}
