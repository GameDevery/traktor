#include "Animation/Editor/AnimationEditorPageFactory.h"
#include "Animation/Editor/AnimationEditorPage.h"
#include "Animation/Animation/Animation.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationEditorPageFactory", AnimationEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet AnimationEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Animation >());
	return typeSet;
}

Ref< editor::IEditorPage > AnimationEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return new AnimationEditorPage(editor);
}

void AnimationEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Animation.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Play"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Forward"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkeleton"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkin"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTrail"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTwistLock"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleIK"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectPreviousBone"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectNextBone"));
}

	}
}
