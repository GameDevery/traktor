/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Button_H
#define traktor_ui_Button_H

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

/*! \brief Button
 * \ingroup UI
 */
class T_DLLCLASS Button : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefaultButton	= WsUser,
		WsToggle = (WsUser << 1)
	};

	Button();

	bool create(Widget* parent, const std::wstring& text, int style = WsNone);

	void setState(bool state);

	bool getState() const;

	virtual Size getPreferedSize() const T_OVERRIDE;

	virtual Size getMaximumSize() const T_OVERRIDE;

private:
	bool m_pushed;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

#endif	// traktor_ui_Button_H
