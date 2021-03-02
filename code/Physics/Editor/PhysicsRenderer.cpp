#include "Physics/JointDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/Editor/IPhysicsJointRenderer.h"
#include "Physics/Editor/IPhysicsShapeRenderer.h"
#include "Physics/Editor/PhysicsRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsRenderer", PhysicsRenderer, Object)

PhysicsRenderer::PhysicsRenderer()
{
	TypeInfoSet jointRendererTypes;
	type_of< IPhysicsJointRenderer >().findAllOf(jointRendererTypes, false);
	for (auto jointRendererType : jointRendererTypes)
	{
		Ref< IPhysicsJointRenderer > jointRenderer = mandatory_non_null_type_cast< IPhysicsJointRenderer* >(jointRendererType->createInstance());
		m_jointRenderers.insert(std::make_pair(&jointRenderer->getDescType(), jointRenderer));
	}

	TypeInfoSet shapeRendererTypes;
	type_of< IPhysicsShapeRenderer >().findAllOf(shapeRendererTypes, false);
	for (auto shapeRendererType : shapeRendererTypes)
	{
		Ref< IPhysicsShapeRenderer > shapeRenderer = mandatory_non_null_type_cast< IPhysicsShapeRenderer* >(shapeRendererType->createInstance());
		m_shapeRenderers.insert(std::make_pair(&shapeRenderer->getDescType(), shapeRenderer));
	}
}

void PhysicsRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	auto it = m_jointRenderers.find(&type_of(jointDesc));
	if (it != m_jointRenderers.end())
		it->second->draw(
			primitiveRenderer,
			jointTransform,
			body1Transform,
			body2Transform,
			jointDesc
		);
}

void PhysicsRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	auto it = m_shapeRenderers.find(&type_of(shapeDesc));
	if (it != m_shapeRenderers.end())
		it->second->draw(
			resourceManager,
			primitiveRenderer,
			body1Transform,
			shapeDesc
		);
}

	}
}
