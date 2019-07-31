#include "Ui/Command.h"
#include "Video/Editor/VideoAsset.h"
#include "Video/Editor/VideoEditorPage.h"
#include "Video/Editor/VideoEditorPageFactory.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.VideoEditorPageFactory", 0, VideoEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet VideoEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< VideoAsset >();
	return typeSet;
}

bool VideoEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{71682019-EB26-234C-8B48-0638F50DA662}"));	// System/Video/Video
	return false;
}

Ref< editor::IEditorPage > VideoEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new VideoEditorPage(editor, site, document);
}

void VideoEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Video.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Video.Editor.Play"));
	outCommands.push_back(ui::Command(L"Video.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Video.Editor.Forward"));
}

	}
}
