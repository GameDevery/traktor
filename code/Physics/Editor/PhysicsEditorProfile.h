#ifndef traktor_physics_SprayEditorProfile_H
#define traktor_physics_SprayEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS PhysicsEditorProfile : public scene::ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createEditorPlugins(
		scene::SceneEditorContext* context,
		RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
	) const;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createControllerEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
	) const;

	virtual void createEntityEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
	) const;

	virtual Ref< world::EntityData > createEntityData(
		scene::SceneEditorContext* context,
		db::Instance* instance
	) const;
};

	}
}

#endif	// traktor_physics_SprayEditorProfile_H
