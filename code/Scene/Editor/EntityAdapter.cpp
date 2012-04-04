#include <algorithm>
#include "Core/Math/Const.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter()
:	m_parent(0)
,	m_selected(false)
,	m_expanded(false)
,	m_visible(true)
,	m_locked(false)
{
}

void EntityAdapter::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

world::EntityData* EntityAdapter::getEntityData() const
{
	return m_entityData;
}

void EntityAdapter::setEntity(world::Entity* entity)
{
	m_entity = entity;
}

world::Entity* EntityAdapter::getEntity() const
{
	return m_entity;
}

std::wstring EntityAdapter::getName() const
{
	return m_entityData ? m_entityData->getName() : L"< Null >";
}

std::wstring EntityAdapter::getTypeName() const
{
	return m_entityData ? type_name(m_entityData) : L"< void >";
}

void EntityAdapter::setTransform0(const Transform& transform)
{
	if (m_entityData)
		m_entityData->setTransform(transform);
}

Transform EntityAdapter::getTransform0() const
{
	return m_entityData ? m_entityData->getTransform() : Transform::identity();
}

void EntityAdapter::setTransform(const Transform& transform)
{
	setTransform0(transform);
	if (m_entity)
		m_entity->setTransform(transform);
}

Transform EntityAdapter::getTransform() const
{
	Transform transform;
	if (m_entity && m_entity->getTransform(transform))
		return transform;
	else
		return getTransform0();
}

Aabb3 EntityAdapter::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb3();
}

bool EntityAdapter::isExternal() const
{
	for (const EntityAdapter* entityAdapter = this; entityAdapter; entityAdapter = entityAdapter->m_parent)
	{
		if (is_a< world::ExternalEntityData >(entityAdapter->getEntityData()))
			return true;
	}
	return false;
}

bool EntityAdapter::isChildOfExternal() const
{
	return m_parent ? m_parent->isExternal() : false;
}

bool EntityAdapter::getExternalGuid(Guid& outGuid) const
{
	if (const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(m_entityData))
	{
		outGuid = externalEntityData->getGuid();
		return true;
	}
	return false;
}

bool EntityAdapter::isGroup() const
{
	return m_entityEditor ? m_entityEditor->isGroup() : false;
}

EntityAdapter* EntityAdapter::getParent() const
{
	return m_parent;
}

EntityAdapter* EntityAdapter::getParentGroup()
{
	EntityAdapter* entity = this;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

EntityAdapter* EntityAdapter::getParentContainerGroup()
{
	EntityAdapter* entity = m_parent;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

void EntityAdapter::addChild(EntityAdapter* child)
{
	if (m_entityEditor->addChildEntity(child))
		link(child);
}

void EntityAdapter::removeChild(EntityAdapter* child)
{
	if (m_entityEditor->removeChildEntity(child))
		unlink(child);
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

void EntityAdapter::link(EntityAdapter* child)
{
	T_ASSERT_M (child->m_parent == 0, L"Child already linked to another parent");
	child->m_parent = this;
	m_children.push_back(child);
}

void EntityAdapter::unlink(EntityAdapter* child)
{
	T_ASSERT (child);
	T_ASSERT_M (child->m_parent == this, L"Entity adapter not child if this");

	RefArray< EntityAdapter >::iterator i = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT (i != m_children.end());
	m_children.erase(i);

	child->m_parent = 0;
}

void EntityAdapter::setEntityEditor(IEntityEditor* entityEditor)
{
	m_entityEditor = entityEditor;
}

IEntityEditor* EntityAdapter::getEntityEditor() const
{
	return m_entityEditor;
}

bool EntityAdapter::isSelected() const
{
	return m_selected;
}

void EntityAdapter::setExpanded(bool expanded)
{
	m_expanded = expanded;
}

bool EntityAdapter::isExpanded() const
{
	return m_expanded;
}

void EntityAdapter::setVisible(bool visible)
{
	m_visible = visible;
}

bool EntityAdapter::isVisible(bool includingParents) const
{
	if (!m_visible)
		return false;

	if (includingParents)
	{
		for (EntityAdapter* parent = m_parent; parent; parent = parent->m_parent)
		{
			if (!parent->m_visible)
				return false;
		}
	}

	return true;
}

void EntityAdapter::setLocked(bool locked)
{
	m_locked = locked;
	if (m_locked)
		m_selected = false;
}

bool EntityAdapter::isLocked(bool includingParents) const
{
	if (m_locked)
		return true;

	if (includingParents)
	{
		for (EntityAdapter* parent = m_parent; parent; parent = parent->m_parent)
		{
			if (parent->m_locked)
				return true;
		}
	}

	return false;
}

AlignedVector< EntityAdapter::SnapPoint > EntityAdapter::getSnapPoints() const
{
	AlignedVector< SnapPoint > snapPoints;

	Transform transform = getTransform();
	Aabb3 boundingBox = m_entity->getBoundingBox();
	if (!boundingBox.empty())
	{
		Vector4 extents[8];
		boundingBox.getExtents(extents);

		const Vector4* normals = Aabb3::getNormals();
		const int* faces = Aabb3::getFaces();

		for (int i = 0; i < 6; ++i)
		{
			Vector4 faceCenter =
				extents[faces[i * 4 + 0]] +
				extents[faces[i * 4 + 1]] +
				extents[faces[i * 4 + 2]] +
				extents[faces[i * 4 + 3]];

			faceCenter /= Scalar(4.0f);

			SnapPoint sp;
			sp.position = transform * faceCenter;
			sp.direction = transform * normals[i];
			snapPoints.push_back(sp);
		}
	}

	return snapPoints;
}

	}
}
