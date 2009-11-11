#ifndef traktor_ui_custom_IpolNodeShape_H
#define traktor_ui_custom_IpolNodeShape_H

#include "Core/Heap/Ref.h"
#include "Ui/Custom/Graph/NodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

		namespace custom
		{

/*! \brief Interpolator node shape.
 * \ingroup UIC
 */
class T_DLLCLASS IpolNodeShape : public NodeShape
{
	T_RTTI_CLASS(IpolNodeShape)

public:
	IpolNodeShape();

	virtual Point getPinPosition(const Node* node, const Pin* pin);

	virtual Ref< Pin > getPinAt(const Node* node, const Point& pt);

	virtual void paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset);

	virtual Size calculateSize(const Node* node);

private:
	Ref< Bitmap > m_imageNode;
	Ref< Bitmap > m_imagePin;
};

		}
	}
}

#endif	// traktor_ui_custom_IpolNodeShape_H
