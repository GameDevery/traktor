#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

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

class MenuItem;
class Point;
class Widget;

/*! Menu
 * \ingroup UI
 */
class T_DLLCLASS Menu : public Object
{
	T_RTTI_CLASS;

public:
	void add(MenuItem* item);

	const RefArray< MenuItem >& getItems() const;

	Ref< Widget > show(Widget* parent, const Point& at) const;

	/*! Show menu.
	 *
	 * This method will not return until an menu item
	 * has been selected.
	 *
	 * \param parent Parent widget.
	 * \param at Position of menu top-left corner, in parent coordinate space.
	 * \param width Optional width of menu, -1 if width should be calculated automatically.
	 * \param maxItems Max number of items visible, scrollbar is automatically added if more items available.
	 * \return Selected menu item, null if menu was cancelled.
	 */
	const MenuItem* showModal(
		Widget* parent,
		const Point& at,
		int32_t width = -1,
		int32_t maxItems = -1
	) const;

private:
	RefArray< MenuItem > m_items;
};

	}
}

