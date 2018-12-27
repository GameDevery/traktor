/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CenterLayout_H
#define traktor_ui_CenterLayout_H

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

/*! \brief Center layout.
 * \ingroup UI
 */
class T_DLLCLASS CenterLayout : public Layout
{
	T_RTTI_CLASS;

public:
	enum CenterAxis
	{
		CaHorizontal = 1,
		CaVertical = 2,
		CaBoth = CaHorizontal | CaVertical
	};

	CenterLayout(uint32_t centerAxis = CaBoth);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	uint32_t m_centerAxis;
};

	}
}

#endif	// traktor_ui_CenterLayout_H
