#ifndef traktor_ui_PopupMenuWx_H
#define traktor_ui_PopupMenuWx_H

#include <vector>
#include <wx/wx.h>
#include "Core/Heap/Ref.h"
#include "Ui/Itf/IPopupMenu.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class PopupMenuWx : public IPopupMenu
{
public:
	PopupMenuWx(EventSubject* owner);

	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);

private:
	wxWindow* m_parent;
	wxMenu m_menu;
	RefArray< MenuItem > m_flatten;
	Ref< MenuItem > m_selected;

	wxMenu* buildSubMenu(MenuItem* parentItem);

	void onMenuSelected(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_PopupMenuWx_H
