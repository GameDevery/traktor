#ifndef traktor_scene_EntityAdapter_H
#define traktor_scene_EntityAdapter_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityData;

	}

	namespace scene
	{

class IEntityEditor;

/*! \brief Entity adapter class.
 *
 * Map between EntityData and Entity.
 * Also keep parent-child relationship of entities.
 */
class T_DLLCLASS EntityAdapter : public Object
{
	T_RTTI_CLASS;

public:
	EntityAdapter(world::EntityData* entityData);

	/*! \name Accessors */
	//@{

	Ref< world::EntityData > getEntityData() const;

	void setEntity(world::Entity* entity);

	Ref< world::Entity > getEntity() const;

	//@}

	/*! \name Information accessors. */
	//@{

	std::wstring getName() const;

	std::wstring getTypeName() const;

	//@}

	/*! \name Spatial entity accessors. */
	//@{

	bool isSpatial() const;

	void setTransform(const Transform& transform);

	Transform getTransform() const;

	Aabb getBoundingBox() const;

	//@}

	/*! \name External entity accessors. */
	//@{

	bool isExternal() const;

	bool isChildOfExternal() const;

	bool getExternalGuid(Guid& outGuid) const;

	//@}

	/*! \name Relationship. */
	//@{

	bool isGroup() const;

	EntityAdapter* getParent() const;

	Ref< EntityAdapter > getParentGroup();

	Ref< EntityAdapter > getParentContainerGroup();

	void addChild(EntityAdapter* child);

	void removeChild(EntityAdapter* child);

	const RefArray< EntityAdapter >& getChildren() const;

	void link(EntityAdapter* child);

	void unlink(EntityAdapter* child);

	//@}

	/*! \name Entity editor. */
	//@{

	void setEntityEditor(IEntityEditor* entityEditor);

	Ref< IEntityEditor > getEntityEditor() const;

	//@}

	/*! \name Entity state. */
	//@{

	bool isSelected() const;

	void setExpanded(bool expanded);

	bool isExpanded() const;

	void setVisible(bool visible);
	
	bool isVisible() const;

	//@}

	/*! \name Editor aid. */
	//@{

	struct SnapPoint
	{
		Vector4 position;
		Vector4 direction;
	};

	AlignedVector< SnapPoint > getSnapPoints() const;

	//@}

private:
	friend class SceneEditorContext;

	Ref< world::EntityData > m_entityData;
	Ref< world::Entity > m_entity;
	EntityAdapter* m_parent;
	RefArray< EntityAdapter > m_children;
	Ref< IEntityEditor > m_entityEditor;
	bool m_selected;
	bool m_expanded;
	bool m_visible;
};

	}
}

#endif	// traktor_scene_EntityAdapter_H
