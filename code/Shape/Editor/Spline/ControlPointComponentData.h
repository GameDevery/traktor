#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class ControlPointComponent;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ControlPointComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ControlPointComponentData() = default;

	Ref< ControlPointComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	float getScale() const { return m_scale; }

private:
	float m_scale = 1.0f;
};

	}
}
