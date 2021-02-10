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

/*! Mini button control.
 * \ingroup UI
 */
class T_DLLCLASS MiniButton : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text);

	bool create(Widget* parent, IBitmap* image);

	void setImage(IBitmap* image);

	virtual Size getPreferedSize() const override;

	virtual Size getMaximumSize() const override;

private:
	bool m_pushed;
	Ref< IBitmap > m_image;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

