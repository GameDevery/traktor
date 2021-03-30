#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "World/Entity.h"
#include "World/Entity/ScriptComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ScriptComponent", ScriptComponent, IEntityComponent)

ScriptComponent::ScriptComponent(const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(nullptr)
,	m_class(clazz)
{
}

void ScriptComponent::destroy()
{
	m_owner = nullptr;
	m_class.clear();
	m_object = nullptr;
	m_methodSetTransform = nullptr;
	m_methodUpdate = nullptr;
}

void ScriptComponent::setOwner(Entity* owner)
{
	T_ASSERT(m_owner == nullptr);
	m_owner = owner;
	m_object = nullptr;
	m_methodSetTransform = nullptr;
	m_methodUpdate = nullptr;
}

void ScriptComponent::setTransform(const Transform& transform)
{
	T_ASSERT(m_owner != nullptr);

	// Check if class has changed, hot-reload new class.
	if (!validate())
		return;

	// Invoke set transform method if available.
	if (m_methodSetTransform != nullptr)
	{
		Any argv[] = { CastAny< Transform >::set(transform) };
		m_methodSetTransform->invoke(m_object, sizeof_array(argv), argv);
	}
}

Aabb3 ScriptComponent::getBoundingBox() const
{
	return Aabb3();
}

void ScriptComponent::update(const UpdateParams& update)
{
	T_ASSERT(m_owner != nullptr);

	// Check if class has changed, hot-reload new class.
	if (!validate())
		return;

	// Invoke update method if available.
	if (m_methodUpdate != nullptr)
	{
		Any argv[] =
		{
			Any::fromObject(update.contextObject),
			Any::fromFloat(update.totalTime),
			Any::fromFloat(update.deltaTime)
		};
		m_methodUpdate->invoke(m_object, sizeof_array(argv), argv);
	}
}

void ScriptComponent::execute(const char* method)
{
	// Check if class has changed, hot-reload new class.
	if (!validate())
		return;

	Ref< const IRuntimeDispatch > dispatch = findRuntimeClassMethod(m_class, method);
	if (dispatch != nullptr)
		dispatch->invoke(m_object, 0, nullptr);
}

bool ScriptComponent::validate()
{
	if (m_class.changed() || m_object == nullptr)
	{
		m_object = createRuntimeClassInstance(m_class, m_owner, 0, nullptr);
		m_methodSetTransform = findRuntimeClassMethod(m_class, "setTransform");
		m_methodUpdate = findRuntimeClassMethod(m_class, "update");
		m_class.consume();
	}
	return (bool)(m_class != nullptr && m_object != nullptr);
}

	}
}
