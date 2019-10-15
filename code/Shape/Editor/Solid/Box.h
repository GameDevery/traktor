#pragma once

#include "Shape/Editor/Solid/IShape.h"

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

/*! Box shape.
 * \ingroup Shape
 */
class T_DLLCLASS Box : public IShape
{
	T_RTTI_CLASS;

public:
	Box();

	virtual Ref< model::Model > createModel(db::Database* database) const override final;

	virtual void createAnchors(AlignedVector< Vector4 >& outAnchors) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_extent;
};

	}
}