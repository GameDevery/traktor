#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FileDialog", FileDialog, ConfigDialog)

bool FileDialog::create(Widget* parent, const std::wstring& key, const std::wstring& title, const std::wstring& filters, const std::wstring& defaultPath, bool save)
{
	if (!ConfigDialog::create(
		parent,
		title,
		dpi96(700),
		dpi96(500),
		ConfigDialog::WsDefaultResizable,
		new TableLayout(L"100%", save ? L"*,100%,*" : L"*,100%", 0, 0)
	))
		return false;

	m_containerPath = new Container();
	m_containerPath->create(this, WsNone, new TableLayout(L"*,*,*,*,*,*,*,*,*", L"100%", dpi96(4), dpi96(4)));

	m_gridFiles = new GridView();
	m_gridFiles->create(this, GridView::WsColumnHeader | WsDoubleBuffer);
	m_gridFiles->addColumn(new GridColumn(L"", dpi96(20)));
	m_gridFiles->addColumn(new GridColumn(L"Filename", dpi96(300)));
	m_gridFiles->addColumn(new GridColumn(L"Size", dpi96(100)));
	m_gridFiles->addColumn(new GridColumn(L"Modified", dpi96(180)));

	// Sort by directory first then name.
	m_gridFiles->setSort([](const GridRow* r1, const GridRow* r2) -> bool {
		auto f1 = r1->getData< File >(L"FILE");
		T_FATAL_ASSERT(f1 != nullptr);
		auto f2 = r2->getData< File >(L"FILE");
		T_FATAL_ASSERT(f2 != nullptr);

		if (f1->isDirectory() && !f2->isDirectory())
			return true;
		if (!f1->isDirectory() && f2->isDirectory())
			return false;

		auto fn1 = f1->getPath().getFileNameNoExtension();
		auto fn2 = f2->getPath().getFileNameNoExtension();
		return compareIgnoreCase(fn1, fn2) < 0;
	});

	m_gridFiles->addEventHandler< SelectionChangeEvent >([&](SelectionChangeEvent* event) {
		auto selectedRow = m_gridFiles->getSelectedRow();
		if (selectedRow)
		{
			auto file = selectedRow->getData< File >(L"FILE");
			T_FATAL_ASSERT(file != nullptr);

			if (!file->isDirectory())
			{
				if (m_editFileName)
					m_editFileName->setText(file->getPath().getFileName());
			}
		}
	});

	m_gridFiles->addEventHandler< GridRowDoubleClickEvent >([&](GridRowDoubleClickEvent* event) {
		auto file = event->getRow()->getData< File >(L"FILE");
		T_FATAL_ASSERT(file != nullptr);

		if (file->isDirectory())
		{
			m_currentPath = file->getPath();
			updatePath();
			updateFiles();
		}
		else
			endModal(DrOk);
	});

	if (save)
	{
		m_editFileName = new Edit();
		m_editFileName->create(this);
	}

	m_bitmapDirectory = new ui::StyleBitmap(L"UI.FileDialog.Directory");
	m_bitmapFile = new ui::StyleBitmap(L"UI.FileDialog.File");
	m_key = key;

	if (!defaultPath.empty())
		m_defaultPath = defaultPath;
	else
		m_defaultPath = FileSystem::getInstance().getCurrentVolumeAndDirectory();

	return true;
}

void FileDialog::destroy()
{
	ConfigDialog::destroy();
}

int32_t FileDialog::showModal(Path& outPath)
{
	std::wstring path = Application::getInstance()->getProperties()->getProperty< std::wstring >(m_key);
	if (!path.empty())
		m_currentPath = path;
	else if (!outPath.empty())
		m_currentPath = outPath.getPathName();
	else
		m_currentPath = m_defaultPath;

	if (m_editFileName)
		m_editFileName->setText(outPath.getFileName());

	updatePath();
	updateFiles();

	m_gridFiles->setMultiSelect(false);

	if (ConfigDialog::showModal() != DrOk)
		return DrCancel;

	if (!m_editFileName)
	{
		auto selectedRow = m_gridFiles->getSelectedRow();
		if (selectedRow == nullptr)
			return DrCancel;

		auto file = selectedRow->getData< File >(L"FILE");
		T_FATAL_ASSERT(file != nullptr);

		if (file->isDirectory())
			return DrCancel;

		outPath = file->getPath();
	}
	else
	{
		if (m_editFileName->getText().empty())
			return DrCancel;

		outPath = m_currentPath.getPathName() + L"/" + m_editFileName->getText();
	}

	Application::getInstance()->getProperties()->setProperty< PropertyString >(m_key, m_currentPath.getPathName());
	return DrOk;
}

int32_t FileDialog::showModal(std::vector< Path >& outPaths)
{
	std::wstring path = Application::getInstance()->getProperties()->getProperty< std::wstring >(m_key);
	if (!path.empty())
		m_currentPath = path;
	else
		m_currentPath = m_defaultPath;

	updatePath();
	updateFiles();

	m_gridFiles->setMultiSelect(true);

	if (ConfigDialog::showModal() != DrOk)
		return DrCancel;

	RefArray< ui::GridRow > rows;
	m_gridFiles->getRows(rows, ui::GridView::GfSelectedOnly);

	for (auto row : rows)
	{
		auto file = row->getData< File >(L"FILE");
		T_FATAL_ASSERT(file != nullptr);

		if (file->isDirectory())
			continue;

		outPaths.push_back(file->getPath());
	}

	Application::getInstance()->getProperties()->setProperty< PropertyString >(m_key, m_currentPath.getPathName());
	return DrOk;
}

void FileDialog::updatePath()
{
	while (m_containerPath->getFirstChild() != nullptr)
		m_containerPath->getFirstChild()->destroy();

#if !defined(_WIN32)
	auto pn = m_currentPath.getPathNameNoVolume();
#else
	auto pn = m_currentPath.getPathName();
#endif

	std::wstring p;
#if !defined(_WIN32)
	p = L"/";
#endif
	for (auto s : StringSplit< std::wstring >(pn, L"/"))
	{
		if (!p.empty())
			p = p + L"/" + s;
		else
			p = s;

		if (!endsWith(s, L":"))
		{
			Ref< Button > buttonPath = new Button();
			buttonPath->create(m_containerPath, s);
			buttonPath->addEventHandler< ButtonClickEvent >([=](ButtonClickEvent* event) {
				m_currentPath = p;
				updatePath();
				updateFiles();
			});
		}
		else
		{
			Ref< DropDown > dropVolume = new DropDown();
			dropVolume->create(m_containerPath);

			for (int32_t i = 0; i < FileSystem::getInstance().getVolumeCount(); ++i)
			{
				std::wstring volumeId = FileSystem::getInstance().getVolumeId(i);
				int32_t index = dropVolume->add(volumeId + L":");
				if (compareIgnoreCase(volumeId + L":", s) == 0)
					dropVolume->select(index);
			}

			dropVolume->addEventHandler< SelectionChangeEvent >([=](SelectionChangeEvent* event) {
				m_currentPath = dropVolume->getSelectedItem();
				updatePath();
				updateFiles();
			});
		}
	}

	m_containerPath->update();
}

void FileDialog::updateFiles()
{
	RefArray< File > files;
	FileSystem::getInstance().find(m_currentPath + L"*.*", files);

	m_gridFiles->removeAllRows();
	for (auto file : files)
	{
		auto fn = file->getPath().getFileName();
		if (fn == L"." || fn == L".." || file->isHidden())
			continue;

		Ref< GridRow > row = new GridRow();
		row->add(new GridItem(file->isDirectory() ? m_bitmapDirectory : m_bitmapFile));
		row->add(new GridItem(fn));

		if (!file->isDirectory())
			row->add(new GridItem(toString(file->getSize())));
		else
			row->add(new GridItem());

		if (!file->isDirectory())
			row->add(new GridItem(
				file->getLastWriteTime().format(L"%c")
			));
		else
			row->add(new GridItem());

		row->setData(L"FILE", file);

		m_gridFiles->addRow(row);
	}

	m_gridFiles->update();
}

	}
}
