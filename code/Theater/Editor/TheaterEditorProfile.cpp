/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEditorProfile.h"
#include "Theater/Editor/TheaterComponentEditorFactory.h"
#include "Ui/Command.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterEditorProfile", 0, TheaterEditorProfile, scene::ISceneEditorProfile)

void TheaterEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Theater.CaptureEntities"));
	outCommands.push_back(ui::Command(L"Theater.DeleteSelectedKey"));
	outCommands.push_back(ui::Command(L"Theater.SetLookAtEntity"));
	outCommands.push_back(ui::Command(L"Theater.EaseVelocity"));
	outCommands.push_back(ui::Command(L"Theater.GotoPreviousKey"));
	outCommands.push_back(ui::Command(L"Theater.GotoNextKey"));
	outCommands.push_back(ui::Command(L"Theater.SplitAct"));
}

void TheaterEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void TheaterEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void TheaterEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void TheaterEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
}

void TheaterEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void TheaterEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
	outComponentEditorFactories.push_back(new TheaterComponentEditorFactory());
}

void TheaterEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void TheaterEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > TheaterEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
