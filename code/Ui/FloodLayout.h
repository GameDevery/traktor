#pragma once

#include "Core/Ref.h"
#include "Ui/Layout.h"
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

/*! \brief Flood layout.
 * \ingroup UI
 */
class T_DLLCLASS FloodLayout : public Layout
{
	T_RTTI_CLASS;

public:
	FloodLayout();

	FloodLayout(const Size& margin);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	Size m_margin;

	Ref< Widget > getFirstVisibleChild(Widget* widget);
};

	}
}

