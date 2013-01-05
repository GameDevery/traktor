#ifndef traktor_ui_Image_H
#define traktor_ui_Image_H

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

class Bitmap;

/*! \brief Static image.
 * \ingroup UI
 */
class T_DLLCLASS Image : public Widget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsTransparent = WsUser
	};

	bool create(Widget* parent, Bitmap* image = 0, int style = WsNone);
	
	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;

	bool setImage(Bitmap* image, bool transparent = false);

	Ref< Bitmap > getImage() const;

	bool isTransparent() const;

private:
	Ref< Bitmap > m_image;
	bool m_transparent;
	
	void eventPaint(Event* event);
};

	}
}

#endif	// traktor_ui_Image_H
