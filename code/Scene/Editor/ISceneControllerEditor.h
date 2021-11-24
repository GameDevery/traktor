#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace ui
	{

class Container;
class Command;

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

/*! Scene controller editor interface.
 * \ingroup Scene
 */
class T_DLLCLASS ISceneControllerEditor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(SceneEditorContext* context, ui::Container* parent) = 0;

	virtual void destroy() = 0;

	virtual void entityRemoved(EntityAdapter* entityAdapter) = 0;

	virtual void propertiesChanged() = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) = 0;
};

	}
}

