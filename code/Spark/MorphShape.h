#pragma once

#include "Spark/Path.h"
#include "Spark/Character.h"
#include "Spark/FillStyle.h"
#include "Spark/LineStyle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

struct SwfShape;
struct SwfStyles;

/*! Flash morph shape.
 * \ingroup Spark
 */
class T_DLLCLASS MorphShape : public Character
{
	T_RTTI_CLASS;

public:
	MorphShape();

	MorphShape(uint16_t id);

	bool create(const Aabb2& shapeBounds, const SwfShape* startShape, const SwfShape* endShape, const SwfStyles* startStyles, const SwfStyles* endStyles);

	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const override final;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	virtual void serialize(ISerializer& s) override final;

private:
	Aabb2 m_shapeBounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FillStyle > m_fillStyles;
	AlignedVector< LineStyle > m_lineStyles;
};

	}
}

