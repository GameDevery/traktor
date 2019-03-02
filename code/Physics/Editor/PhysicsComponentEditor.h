#pragma once

#include "Physics/Editor/PhysicsRenderer.h"
#include "Scene/Editor/IComponentEditor.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

	}

	namespace world
	{

class IEntityComponentData;

	}

	namespace physics
	{

class PhysicsComponentEditor : public scene::IComponentEditor
{
	T_RTTI_CLASS;

public:
	PhysicsComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	scene::SceneEditorContext* m_context;
	scene::EntityAdapter* m_entityAdapter;
	world::IEntityComponentData* m_componentData;
	PhysicsRenderer m_physicsRenderer;
};

	}
}

