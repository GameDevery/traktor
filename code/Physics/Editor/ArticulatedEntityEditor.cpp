#include "Core/Math/Const.h"
#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntityEditor", ArticulatedEntityEditor, scene::DefaultEntityEditor)

ArticulatedEntityEditor::ArticulatedEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

void ArticulatedEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	ArticulatedEntityData* articulatedEntityData = checked_type_cast< ArticulatedEntityData* >(getEntityAdapter()->getEntityData());
	if (!articulatedEntityData)
		return;

	if (getContext()->shouldDrawGuide(L"Physics.Joints"))
	{
		const RefArray< scene::EntityAdapter >& constraintChildren = getEntityAdapter()->getChildren();
		const auto& constraints = articulatedEntityData->getConstraints();
		Transform transform = articulatedEntityData->getTransform();

		for (uint32_t i = 0; i < (uint32_t)constraints.size(); ++i)
		{
			const ArticulatedEntityData::Constraint& constraint = constraints[i];
			if (constraint.entityIndex1 < 0)
				continue;

			scene::EntityAdapter* entity1 = constraintChildren[constraint.entityIndex1];
			if (!entity1)
				continue;

			scene::EntityAdapter* entity2 = (constraint.entityIndex2 >= 0) ? constraintChildren[constraint.entityIndex2] : 0;
			if (entity2)
			{
				Transform body1Transform0 = entity1->getTransform0();
				Transform body1Transform = entity1->getTransform();
				Transform body2Transform0 = entity2->getTransform0();
				Transform body2Transform = entity2->getTransform();

				m_physicsRenderer.draw(
					primitiveRenderer,
					body1Transform0,
					body1Transform,
					body2Transform0,
					body2Transform,
					constraint.jointDesc
				);
			}
			else
			{
				Transform body1Transform0 = entity1->getTransform0();
				Transform body1Transform = entity1->getTransform();

				m_physicsRenderer.draw(
					primitiveRenderer,
					body1Transform0,
					body1Transform,
					constraint.jointDesc
				);
			}
		}
	}
}

	}
}
