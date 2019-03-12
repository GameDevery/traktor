#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/Editor/PhysicsEntityEditorFactory.h"
#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/RigidEntityData.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsEntityEditorFactory", PhysicsEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet PhysicsEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ArticulatedEntityData >();
	typeSet.insert< RigidEntityData >();
	return typeSet;
}

Ref< scene::IEntityEditor > PhysicsEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	if (is_type_of< ArticulatedEntityData >(entityDataType))
		return new ArticulatedEntityEditor(context, entityAdapter);
	else if (is_type_of< RigidEntityData >(entityDataType))
		return new RigidEntityEditor(context, entityAdapter);

	return 0;
}

	}
}
