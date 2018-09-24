/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_AspectLayout_H
#define traktor_ui_AspectLayout_H

#include "Ui/Layout.h"

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

/*! \brief Aspect layout.
 * \ingroup UI
 */
class T_DLLCLASS AspectLayout : public Layout
{
	T_RTTI_CLASS;

public:
	AspectLayout(float ratio = -1.0f);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) T_OVERRIDE;

	virtual void update(Widget* widget) T_OVERRIDE;

private:
	float m_ratio;
};

	}
}

#endif	// traktor_ui_AspectLayout_H
