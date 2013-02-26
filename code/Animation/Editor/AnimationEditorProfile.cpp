#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Cloth/ClothEntityFactory.h"
#include "Animation/Cloth/ClothEntityRenderer.h"
#include "Animation/Editor/AnimationEditorProfile.h"
#include "Animation/Editor/AnimationEntityEditorFactory.h"
#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Core/Serialization/ISerializable.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationEditorProfile", 0, AnimationEditorProfile, scene::ISceneEditorProfile)

void AnimationEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Animation.Editor.StepForward"));
	outCommands.push_back(ui::Command(L"Animation.Editor.StepBack"));
	outCommands.push_back(ui::Command(L"Animation.Editor.GotoPreviousKey"));
	outCommands.push_back(ui::Command(L"Animation.Editor.GotoNextKey"));
	outCommands.push_back(ui::Command(L"Animation.Editor.InsertKey"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Reset"));
}

void AnimationEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void AnimationEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new AnimationFactory(context->getResourceDatabase()));
}

void AnimationEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new AnimatedMeshEntityFactory(context->getResourceManager(), context->getPhysicsManager()));
	outEntityFactories.push_back(new ClothEntityFactory(context->getResourceManager(), context->getRenderSystem()));
	outEntityFactories.push_back(new PathEntityFactory());
}

void AnimationEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new ClothEntityRenderer());
	outEntityRenderers.push_back(new PathEntityRenderer());
}

void AnimationEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void AnimationEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new AnimationEntityEditorFactory());
}

Ref< world::EntityData > AnimationEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
