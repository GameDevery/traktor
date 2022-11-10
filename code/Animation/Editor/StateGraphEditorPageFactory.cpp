/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/StateGraph.h"
#include "Animation/Editor/StateGraphEditorPage.h"
#include "Animation/Editor/StateGraphEditorPageFactory.h"
#include "Core/Serialization/DeepClone.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateGraphEditorPageFactory", 0, StateGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet StateGraphEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< StateGraph >();
	return typeSet;
}

bool StateGraphEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > StateGraphEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new StateGraphEditorPage(editor, site, document);
}

void StateGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EventSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.BrowseMesh"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.BrowseSkeleton"));
}

Ref< ISerializable > StateGraphEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
