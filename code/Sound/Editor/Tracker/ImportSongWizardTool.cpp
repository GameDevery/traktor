#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "Sound/Editor/Tracker/ImportMod.h"
#include "Sound/Editor/Tracker/ImportSongWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.ImportSongWizardTool", 0, ImportSongWizardTool, editor::IWizardTool)

std::wstring ImportSongWizardTool::getDescription() const
{
	return L"Import MOD/XM/S3M..."; // i18n::Text(L"IMPORT_SONG_WIZARDTOOL_DESCRIPTION");
}

uint32_t ImportSongWizardTool::getFlags() const
{
	return WfGroup;
}

bool ImportSongWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Path pathName;

	ui::FileDialog fileDialog;
	fileDialog.create(parent, L"Select MOD to import...", L"All files;*.*");
	fileDialog.showModal(pathName);
	fileDialog.destroy();

	std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	std::wstring samplePath = group->getPath();

	if (!FileSystem::getInstance().makeAllDirectories(assetPath + L"/" + samplePath))
		return false;

	ImportMod().import(pathName, assetPath, samplePath, group);

	return true;
}

	}
}