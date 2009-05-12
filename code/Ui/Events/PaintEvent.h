#ifndef traktor_ui_PaintEvent_H
#define traktor_ui_PaintEvent_H

#include "Ui/Event.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
	
class Canvas;

/*! \brief Paint event.
 * \ingroup UI
 */
class T_DLLCLASS PaintEvent : public Event
{
	T_RTTI_CLASS(PaintEvent)
	
public:
	PaintEvent(EventSubject* sender, Object* item, Canvas& canvas, const Rect& rc);
	
	Canvas& getCanvas() const;
	
	const Rect& getUpdateRect() const;
	
private:
	Canvas& m_canvas;
	Rect m_rc;
};
	
	}
}

#endif	// traktor_ui_PaintEvent_H
