#ifndef traktor_ui_DropDownWx_H
#define traktor_ui_DropDownWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IDropDown.h"

namespace traktor
{
	namespace ui
	{

class DropDownWx : public WidgetWxImpl< IDropDown, wxComboBox >
{
public:
	DropDownWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual int getSelected() const;

private:
	void onSelected(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_DropDownWx_H
