#pragma once

#include "Core/Object.h"
#include "Ui/Size.h"

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

class Widget;

/*! Layout base class.
 * \ingroup UI
 */
class T_DLLCLASS Layout : public Object
{
	T_RTTI_CLASS;

public:
	/*!
	 * Calculate tight fit bound from loose bounds.
	 * Used for automatically resize dialogs for best fit.
	 */
	virtual bool fit(Widget* widget, const Size& bounds, Size& result) = 0;

	/*!
	 */
	virtual void update(Widget* widget) = 0;
};

	}
}

