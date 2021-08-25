#include "Animation/AnimationEntityFactory.h"
#include "Animation/AnimatedMeshComponentRenderer.h"
#include "Animation/AnimationResourceFactory.h"
#include "Animation/Boids/BoidsRenderer.h"
#include "Animation/Cloth/ClothRenderer.h"
#include "Animation/Editor/AnimationEditorProfile.h"
#include "Animation/Editor/AnimatedMeshComponentEditorFactory.h"
#include "Animation/Editor/Cloth/ClothEntityEditorFactory.h"
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
}

void AnimationEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Animation.Cloth");
	outIds.insert(L"Animation.Path");
	outIds.insert(L"Animation.Skeleton.Bind");
	outIds.insert(L"Animation.Skeleton.Pose");
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
	outResourceFactories.push_back(new AnimationResourceFactory());
}

void AnimationEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new AnimationEntityFactory(context->getResourceManager(), context->getRenderSystem(), context->getPhysicsManager()));
}

void AnimationEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new AnimatedMeshComponentRenderer());
	outEntityRenderers.push_back(new BoidsRenderer());
	outEntityRenderers.push_back(new ClothRenderer());
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
	outEntityEditorFactories.push_back(new ClothEntityEditorFactory());
}

void AnimationEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new AnimatedMeshComponentEditorFactory());
}

Ref< world::EntityData > AnimationEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

	}
}
