#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMorphShape.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/Path.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashMorphShape", 0, FlashMorphShape, FlashCharacter)

FlashMorphShape::FlashMorphShape()
{
}

FlashMorphShape::FlashMorphShape(uint16_t id)
:	FlashCharacter(id)
{
	m_shapeBounds.min.x = m_shapeBounds.max.x =
	m_shapeBounds.min.y = m_shapeBounds.max.y = 0.0f;
}

bool FlashMorphShape::create(const SwfRect& shapeBounds, const SwfShape* startShape, const SwfShape* endShape, const SwfStyles* startStyles, const SwfStyles* endStyles)
{
	uint16_t fillStyle0 = 0;
	uint16_t fillStyle1 = 0;
	uint16_t lineStyle = 0;
	uint32_t fillStyleBase = 0;
	uint32_t lineStyleBase = 0;

	m_fillStyles.resize(startStyles->numFillStyles);
	for (uint32_t i = 0; i < startStyles->numFillStyles; ++i)
		m_fillStyles[i].create(startStyles->fillStyles[i]);

	m_lineStyles.resize(startStyles->numLineStyles);
	for (uint32_t i = 0; i < startStyles->numLineStyles; ++i)
		m_lineStyles[i].create(startStyles->lineStyles[i]);

	Path path;

	for (uint16_t i = 0; i < startShape->numShapeRecords; ++i)
	{
		SwfShapeRecord* shapeRecord = startShape->shapeRecords[i];
		if (shapeRecord->edgeFlag && shapeRecord->edge.straightFlag)
		{
			const SwfStraightEdgeRecord& s = shapeRecord->edge.straightEdge;
			if (s.generalLineFlag)
				path.lineTo(float(s.deltaX), float(s.deltaY), Path::CmRelative);
			else
			{
				if (!s.vertLineFlag)
					path.lineTo(float(s.deltaX), 0.0f, Path::CmRelative);
				else
					path.lineTo(0.0f, float(s.deltaY), Path::CmRelative);
			}
		}
		else if (shapeRecord->edgeFlag && !shapeRecord->edge.straightFlag)
		{
			const SwfCurvedEdgeRecord& c = shapeRecord->edge.curvedEdge;
			path.quadraticTo(
				float(c.controlDeltaX),
				float(c.controlDeltaY),
				float(c.controlDeltaX + c.anchorDeltaX),
				float(c.controlDeltaY + c.anchorDeltaY),
				Path::CmRelative
			);
		}
		else if (!shapeRecord->edgeFlag)
		{
			// Whenever a style records appear we close the current sub path.
			path.end(
				fillStyle0 ? fillStyle0 + fillStyleBase : 0,
				fillStyle1 ? fillStyle1 + fillStyleBase : 0,
				lineStyle ? lineStyle + lineStyleBase : 0
			);

			const SwfStyleRecord& s = shapeRecord->style;
			if (s.stateMoveTo)
			{
				path.moveTo(
					float(s.moveDeltaX),
					float(s.moveDeltaY),
					Path::CmAbsolute
				);
			}

			if (s.stateNewStyles)
			{
				fillStyleBase = uint32_t(m_fillStyles.size());
				lineStyleBase = uint32_t(m_lineStyles.size());

				m_fillStyles.resize(fillStyleBase + s.newStyles->numFillStyles);
				m_lineStyles.resize(lineStyleBase + s.newStyles->numLineStyles);

				for (int j = 0; j < s.newStyles->numFillStyles; ++j)
				{
					if (!m_fillStyles[fillStyleBase + j].create(s.newStyles->fillStyles[j]))
						return false;
				}
				for (int j = 0; j < s.newStyles->numLineStyles; ++j)
				{
					if (!m_lineStyles[lineStyleBase + j].create(s.newStyles->lineStyles[j]))
						return false;
				}

				fillStyle0 =
				fillStyle1 =
				lineStyle = 0;

				m_paths.push_back(path);
				path.reset();
			}

			if (s.stateFillStyle0)
				fillStyle0 = s.fillStyle0;
			if (s.stateFillStyle1)
				fillStyle1 = s.fillStyle1;
			if (s.stateLineStyle)
				lineStyle = s.lineStyle;
		}
	}

	path.end(
		fillStyle0 ? fillStyle0 + fillStyleBase : 0,
		fillStyle1 ? fillStyle1 + fillStyleBase : 0,
		lineStyle ? lineStyle + lineStyleBase : 0
	);
	m_paths.push_back(path);

	m_shapeBounds = shapeBounds;

	return true;
}

Ref< FlashCharacterInstance > FlashMorphShape::createInstance(
	ActionContext* context,
	FlashCharacterInstance* parent,
	const std::string& name,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashMorphShapeInstance(context, parent, this);
}

bool FlashMorphShape::serialize(ISerializer& s)
{
	if (!FlashCharacter::serialize(s))
		return false;

	s >> MemberSwfRect(L"shapeBounds", m_shapeBounds);
	s >> MemberStlList< Path, MemberComposite< Path > >(L"paths", m_paths);
	s >> MemberAlignedVector< FlashFillStyle, MemberComposite< FlashFillStyle > >(L"fillStyles", m_fillStyles);
	s >> MemberAlignedVector< FlashLineStyle, MemberComposite< FlashLineStyle > >(L"lineStyles", m_lineStyles);

	return true;
}

	}
}
