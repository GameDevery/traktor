/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ColorEvent_H
#define traktor_ui_ColorEvent_H

#include "Core/Math/Color4ub.h"
#include "Ui/Event.h"

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

/*! \brief Color change event.
 * \ingroup UI
 */
class T_DLLCLASS ColorEvent : public Event
{
	T_RTTI_CLASS;

public:
	ColorEvent(EventSubject* sender, const Color4ub& color);

	const Color4ub& getColor() const;

private:
	Color4ub m_color;
};

	}
}

#endif	// traktor_ui_ColorEvent_H
