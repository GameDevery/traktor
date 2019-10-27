#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Solid/SolidTypes.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class Model;

	}

    namespace shape
    {

class PrimitiveEntityData;

/*! Primitive entity.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveEntity : public world::Entity
{
    T_RTTI_CLASS;

public:
    PrimitiveEntity(const PrimitiveEntityData* data, const Transform& transform);

    virtual void setTransform(const Transform& transform) override final;

    virtual bool getTransform(Transform& outTransform) const override final;

	virtual Aabb3 getBoundingBox() const override final;

    virtual void update(const world::UpdateParams& update) override final;

	const PrimitiveEntityData* getData() const { return m_data; }

    const Transform& getTransform() const { return m_transform; }

    const model::Model* getModel() const { return m_model; }

    void setSelectedMaterial(uint32_t selectedMaterial) { m_selectedMaterial = selectedMaterial; }

    uint32_t getSelectedMaterial() const { return m_selectedMaterial; }

    bool isDirty() const { return m_dirty; }

    void resetDirty() { m_dirty = false; }

protected:
    friend class PrimitiveEntityData;

	Ref< const PrimitiveEntityData > m_data;
    Transform m_transform;
	Ref< const model::Model > m_model;
	Aabb3 m_boundingBox;
    uint32_t m_selectedMaterial;
    bool m_dirty;
};

    }
}