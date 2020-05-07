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
	namespace ui
	{

/*! Horizontal or vertical splitter with multiple split panes.
 * \ingroup UI
 */
class T_DLLCLASS MultiSplitter : public Widget
{
	T_RTTI_CLASS;

public:
	MultiSplitter();

	/*! Create splitter control.
	 *
	 * \param parent Parent widget.
	 * \param vertical Orientation of splitters.
	 */
	bool create(Widget* parent, bool vertical = true);

	virtual void update(const Rect* rc = 0, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferedSize() const override;

	virtual Size getMaximumSize() const override;

private:
	bool m_vertical;
    AlignedVector< float > m_splitters;

    void eventChild(ChildEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}
