/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_PhysicsComponentEditorFactory_H
#define traktor_physics_PhysicsComponentEditorFactory_H

#include "Scene/Editor/IComponentEditorFactory.h"

namespace traktor
{
	namespace physics
	{

/*! \brief
 */
class PhysicsComponentEditorFactory : public scene::IComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const override final;

	virtual Ref< scene::IComponentEditor > createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const override final;
};

	}
}

#endif	// traktor_physics_PhysicsComponentEditorFactory_H
