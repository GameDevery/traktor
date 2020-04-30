#pragma once

#include "Scene/Editor/ISceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS AnimationEditorProfile : public scene::ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const override final;

	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const override final;

	virtual void createEditorPlugins(
		scene::SceneEditorContext* context,
		RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
	) const override final;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const override final;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const override final;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const override final;

	virtual void createControllerEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
	) const override final;

	virtual void createEntityEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
	) const override final;

	virtual void createComponentEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
	) const override final;

	virtual Ref< world::EntityData > createEntityData(
		scene::SceneEditorContext* context,
		db::Instance* instance
	) const override final;
};

	}
}

