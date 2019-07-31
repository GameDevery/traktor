#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Editor/BatchDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/Splitter.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/PropertyList/AutoPropertyList.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BatchDialog", BatchDialog, ui::ConfigDialog)

BatchDialog::BatchDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool BatchDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"FLASH_BATCH_DIALOG_TITLE"),
		ui::dpi96(900),
		ui::dpi96(500),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, ui::dpi96(200));

	Ref< ui::Container > textureListContainer = new ui::Container();
	textureListContainer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::ToolBar > movieListTools = new ui::ToolBar();
	if (!movieListTools->create(textureListContainer))
		return false;

	movieListTools->addImage(new ui::StyleBitmap(L"Flash.PlusMinus"), 2);
	movieListTools->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_BATCH_ADD"), 0, ui::Command(L"FlashBatch.Add")));
	movieListTools->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_BATCH_REMOVE"), 1, ui::Command(L"FlashBatch.Remove")));
	movieListTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &BatchDialog::eventTextureListToolClick);

	m_movieList = new ui::ListBox();
	m_movieList->create(textureListContainer, ui::ListBox::WsExtended);
	m_movieList->addEventHandler< ui::SelectionChangeEvent >(this, &BatchDialog::eventTextureListSelect);

	m_moviePropertyList = new ui::AutoPropertyList();
	m_moviePropertyList->create(splitter, ui::WsDoubleBuffer | ui::AutoPropertyList::WsColumnHeader);
	m_moviePropertyList->setSeparator(ui::dpi96(200));
	m_moviePropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_moviePropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));

	return true;
}

void BatchDialog::destroy()
{
	ui::ConfigDialog::destroy();
}

bool BatchDialog::showModal(RefArray< MovieAsset >& outAssets)
{
	if (ui::ConfigDialog::showModal() != ui::DrOk)
		return false;

	m_moviePropertyList->apply();

	for (int i = 0; i < m_movieList->count(); ++i)
		outAssets.push_back(checked_type_cast< MovieAsset* >(m_movieList->getData(i)));

	return true;
}

void BatchDialog::addTexture()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"FLASH_BATCH_FILE_TITLE"), L"All files;*.*"))
		return;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	for (std::vector< Path >::iterator i = fileNames.begin(); i != fileNames.end(); ++i)
	{
		Path texturePath;
		if (!FileSystem::getInstance().getRelativePath(
			FileSystem::getInstance().getAbsolutePath(*i),
			FileSystem::getInstance().getAbsolutePath(assetPath),
			texturePath
		))
			texturePath = *i;

		Ref< MovieAsset > asset = new MovieAsset();
		asset->setFileName(texturePath);

		m_movieList->add(
			i->getFileName(),
			asset
		);
	}
}

void BatchDialog::removeTexture()
{
	m_moviePropertyList->bind(0);
	m_moviePropertyList->update();

	for (int index = m_movieList->getSelected(); index >= 0; index = m_movieList->getSelected())
		m_movieList->remove(index);
}

void BatchDialog::eventTextureListToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"FlashBatch.Add")
		addTexture();
	else if (cmd == L"FlashBatch.Remove")
		removeTexture();
}

void BatchDialog::eventTextureListSelect(ui::SelectionChangeEvent* event)
{
	Ref< MovieAsset > asset = checked_type_cast< MovieAsset* >(m_movieList->getSelectedData());
	m_moviePropertyList->apply();
	m_moviePropertyList->bind(asset);
	m_moviePropertyList->update();
}

	}
}
