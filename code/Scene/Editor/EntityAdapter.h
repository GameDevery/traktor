#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"

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

	namespace world
	{

class Entity;
class EntityData;
class IEntityComponent;
class IEntityComponentData;
class ILayerAttribute;

	}

	namespace scene
	{

class IComponentEditor;
class IEntityEditor;
class SceneEditorContext;

/*! Entity adapter class.
 *
 * Map between EntityData and Entity.
 * Also keep parent-child relationship of entities.
 */
class T_DLLCLASS EntityAdapter : public Object
{
	T_RTTI_CLASS;

public:
	explicit EntityAdapter(SceneEditorContext* context);

	/*! Prepare adapter.
	 *
	 * Prepare adapter for editor use.
	 */
	void prepare(
		world::EntityData* entityData,
		world::Entity* entity,
		uint32_t hash
	);

	/*! \name Accessors */
	//@{

	world::EntityData* getEntityData() const;

	world::Entity* getEntity() const;

	RefArray< world::IEntityComponent > getComponents() const;

	world::IEntityComponentData* getComponentData(const TypeInfo& componentDataType) const;

	world::IEntityComponent* getComponent(const TypeInfo& componentType) const;

	template < typename ComponentDataType >
	ComponentDataType* getComponentData() const
	{
		return checked_type_cast< ComponentDataType* >(getComponentData(type_of< ComponentDataType >()));
	}

	template < typename ComponentType >
	ComponentType* getComponent() const
	{
		return checked_type_cast< ComponentType* >(getComponent(type_of< ComponentType >()));
	}

	void dropHash();

	uint32_t getHash() const;

	//@}

	/*! \name Information accessors. */
	//@{

	const Guid& getId() const;

	std::wstring getName() const;

	std::wstring getPath() const;

	std::wstring getTypeName() const;

	//@}

	/*! \name Spatial accessors. */
	//@{

	void setTransform0(const Transform& transform);

	Transform getTransform0() const;

	void setTransform(const Transform& transform);

	Transform getTransform() const;

	Aabb3 getBoundingBox() const;

	//@}

	/*! \name External entity accessors. */
	//@{

	bool isExternal() const;

	bool isChildOfExternal() const;

	bool getExternalGuid(Guid& outGuid) const;

	//@}

	/*! \name Layer entity accessors. */
	//@{

	bool isLayer() const;

	const world::ILayerAttribute* getLayerAttribute(const TypeInfo& attributeType) const;

	template < typename AttributeType >
	const AttributeType* getLayerAttribute() const
	{
		return checked_type_cast< const AttributeType* >(getLayerAttribute(type_of< AttributeType >()));
	}

	//@}

	/*! \name Relationship. */
	//@{

	bool isChildrenPrivate() const;

	bool isPrivate() const;

	bool isGroup() const;

	EntityAdapter* getParent() const;

	EntityAdapter* getParentGroup();

	EntityAdapter* getParentContainerGroup();

	void addChild(EntityAdapter* child);

	void removeChild(EntityAdapter* child);

	void swapChildren(EntityAdapter* child1, EntityAdapter* child2);

	const RefArray< EntityAdapter >& getChildren() const;

	EntityAdapter* findChildAdapterFromEntity(const world::Entity* entity) const;

	void link(EntityAdapter* child);

	void unlinkChild(EntityAdapter* child);

	void unlinkAllChildren();

	void unlinkFromParent();

	//@}

	/*! \name Entity/component editor. */
	//@{

	IEntityEditor* getEntityEditor() const;

	//@}

	/*! \name Entity state. */
	//@{

	bool isSelected() const;

	void setExpanded(bool expanded);

	bool isExpanded() const;

	void setVisible(bool visible);

	bool isVisible(bool includingParents = true) const;

	void setLocked(bool locked);

	bool isLocked(bool includingParents = true) const;

	//@}

	/*! \name Editor aid. */
	//@{

	struct SnapPoint
	{
		Vector4 position;
		Vector4 direction;
	};

	AlignedVector< SnapPoint > getSnapPoints() const;

	void drawGuides(render::PrimitiveRenderer* primitiveRenderer) const;

	//@}

private:
	friend class SceneEditorContext;

	SceneEditorContext* m_context;
	const TypeInfo* m_entityDataType;
	Ref< world::EntityData > m_entityData;
	Ref< world::Entity > m_entity;
	uint32_t m_hash;
	EntityAdapter* m_parent;
	RefArray< EntityAdapter > m_children;
	SmallMap< const world::Entity*, EntityAdapter* > m_childMap;
	Ref< IEntityEditor > m_entityEditor;
	RefArray< IComponentEditor > m_componentEditors;
	bool m_selected;
	bool m_expanded;
	bool m_visible;
	bool m_locked;
};

	}
}

