#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class Bitmap;

/*! \brief Color gradient control.
 * \ingroup UI
 */
class T_DLLCLASS ColorGradientControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style, const Color4ub& color);

	virtual Size getPreferedSize() const override;

	void setColor(const Color4ub& color, bool updateCursor);

	Color4ub getColor() const;

private:
	float m_hue;
	Point m_cursor;
	Ref< drawing::Image > m_gradientImage;
	Ref< Bitmap > m_gradientBitmap;

	void updateGradientImage();

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

