#include "Animation/AnimationResourceFactory.h"
#include "Animation/Skeleton.h"
#include "Animation/Pose.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Animation.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationResourceFactory", AnimationResourceFactory, resource::IResourceFactory)

const TypeInfoSet AnimationResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< StateGraph >();
	typeSet.insert< Animation >();
	typeSet.insert< Skeleton >();
	typeSet.insert< Pose >();
	return typeSet;
}

const TypeInfoSet AnimationResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet(resourceType);
}

bool AnimationResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AnimationResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< Object > object = instance->getObject();
	if (StateGraph* stateGraph = dynamic_type_cast< StateGraph* >(object))
	{
		// Ensure state node resources are loaded as well.
		const RefArray< StateNode >& states = stateGraph->getStates();
		for (auto state : states)
		{
			if (!state->bind(resourceManager))
				return nullptr;
		}
	}
	return object;
}

	}
}
